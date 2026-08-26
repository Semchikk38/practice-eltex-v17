#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <poll.h>
#include <errno.h>
#include <time.h>

#define MAX_DRIVERS 100
#define MAX_MSG 256
#define SHM_NAME "/taxi_shm"

struct driver_status { // Статус Драйвера
    pid_t pid;
    int busy; 
    int remaining;
    int active;
};

struct shared_mem { // Структура разделяемой памяти
    struct driver_status drivers[MAX_DRIVERS];
    int count;
};

struct task_cmd { // Структура команд, отправляемые драйверу
    int task_timer;
};

struct driver_info { // Информация о драйвере
    pid_t pid;
    int cmd_fd_write;
    int resp_fd_read;
    int slot;
};

static struct shared_mem *shm = NULL;
static struct driver_info drivers_info[MAX_DRIVERS];
static int num_drivers = 0;

// Находим индекс драйвера в разделяемой памяти по его PID
int find_driver_slot_by_pid(pid_t pid) {
    for (int i = 0; i < shm->count; i++) {
        if (shm->drivers[i].active && shm->drivers[i].pid == pid) {
            return i;
        }
    }
    return -1;
}
// Выполняется в дочернем процессе
void driver_main(int cmd_fd_read, int resp_fd_write, int slot) {
    // Инициализация статуса (поля pid и active уже установлены родителем)
    shm->drivers[slot].busy = 0;
    shm->drivers[slot].remaining = 0;

    // Создание timerfd для таймеров
    int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timerfd < 0) {
        perror("timerfd_create");
        exit(EXIT_FAILURE);
    }

    struct pollfd fds[2];
    fds[0].fd = cmd_fd_read;
    fds[0].events = POLLIN;
    fds[1].fd = timerfd;
    fds[1].events = POLLIN;

    printf("Driver %d started\n", getpid());

    while (1) {
        int ret = poll(fds, 2, -1);  // ожидание событий
        if (ret < 0) {
            if (errno == EINTR) continue;
            perror("poll");
            break;
        }

        // Команда от CLI, обрабатываем команды
        if (fds[0].revents & POLLIN) {
            struct task_cmd cmd;
            ssize_t n = read(cmd_fd_read, &cmd, sizeof(cmd));
            if (n <= 0) {
                if (n == 0) break;  // Канал закрыт
                if (errno == EINTR) continue; // Прервано сигналом
                perror("read cmd");
                break;
            }

            if (shm->drivers[slot].busy) { // Проверяем не занят ли драйвер
                // Отправка ошибки занятости
                char msg[MAX_MSG];
                int len = snprintf(msg, sizeof(msg), "Busy %d\n", shm->drivers[slot].remaining);
                if (write(resp_fd_write, msg, len) < 0) {
                    perror("write busy");
                }
            } else {
                // Драйвер свободен - принимаем задачу
                shm->drivers[slot].busy = 1;
                shm->drivers[slot].remaining = cmd.task_timer;

                // Настройка таймера
                struct itimerspec its;
                its.it_value.tv_sec = cmd.task_timer;
                its.it_value.tv_nsec = 0;
                its.it_interval.tv_sec = 0;
                its.it_interval.tv_nsec = 0;
                // Установка таймера
                if (timerfd_settime(timerfd, 0, &its, NULL) < 0) {
                    perror("timerfd_settime");
                }

                // Подтверждение приёма
                const char *ok_msg = "OK\n";
                if (write(resp_fd_write, ok_msg, strlen(ok_msg)) < 0) {
                    perror("write ok");
                }
            }
        }

        // Истечение таймера
        if (fds[1].revents & POLLIN) {
            uint64_t expirations;
            ssize_t n = read(timerfd, &expirations, sizeof(expirations));
            if (n < 0 && errno != EAGAIN) {
                perror("read timerfd");
            } else {
                // Задача выполнена - сбрасываем статус
                shm->drivers[slot].busy = 0;
                shm->drivers[slot].remaining = 0;
                printf("Driver %d: task completed, available\n", getpid());
            }
        }
    }

    close(cmd_fd_read);
    close(resp_fd_write);
    close(timerfd);
    _exit(0);
}
// Создание нового процесса-драйвера
void create_driver() {
    // Проверка не достигнут ли лимит
    if (num_drivers >= MAX_DRIVERS || shm->count >= MAX_DRIVERS) {
        printf("Maximum number of drivers reached\n");
        return;
    }

    int cmd_pipe[2], resp_pipe[2];
    if (pipe(cmd_pipe) < 0 || pipe(resp_pipe) < 0) {
        perror("pipe");
        return;
    }

    int slot = shm->count;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    if (pid == 0) {
        // Дочерний процесс (драйвер)
        close(cmd_pipe[1]);
        close(resp_pipe[0]);
        driver_main(cmd_pipe[0], resp_pipe[1], slot);
        _exit(EXIT_SUCCESS);
    } else {
        // Родитель (CLI)
        close(cmd_pipe[0]);
        close(resp_pipe[1]);

        // Инициализация записи в разделяемой памяти
        shm->drivers[slot].pid = pid;
        shm->drivers[slot].busy = 0;
        shm->drivers[slot].remaining = 0;
        shm->drivers[slot].active = 1;
        shm->count++;

        // Сохраняем информацию о драйвере в родительском процессе
        drivers_info[num_drivers].pid = pid;
        drivers_info[num_drivers].cmd_fd_write = cmd_pipe[1];
        drivers_info[num_drivers].resp_fd_read = resp_pipe[0];
        drivers_info[num_drivers].slot = slot;
        num_drivers++;

        printf("Driver created with PID %d\n", pid);
    }
}
// Отправка задачи драйверу
void send_task(pid_t pid, int task_timer) {
    int idx = -1; // Ищем драйвер по PID
    for (int i = 0; i < num_drivers; i++) {
        if (drivers_info[i].pid == pid) {
            idx = i;
            break;
        }
    }
    if (idx == -1) {
        printf("Driver with PID %d not found\n", pid);
        return;
    }

    // Формируем и отправляем команду
    struct task_cmd cmd;
    cmd.task_timer = task_timer;
    if (write(drivers_info[idx].cmd_fd_write, &cmd, sizeof(cmd)) < 0) {
        perror("write task command");
        return;
    }

    // Ожидание ответа с таймаутом
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(drivers_info[idx].resp_fd_read, &readfds);
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    // select ждёт, пока канал не станет доступен для чтения
    int ready = select(drivers_info[idx].resp_fd_read + 1, &readfds, NULL, NULL, &tv);
    if (ready < 0) {
        if (errno == EINTR) return; // Прервано сигналом
        perror("select");
        return;
    } else if (ready == 0) {
        printf("No response from driver %d\n", pid);
        return;
    }

    char buffer[MAX_MSG];
    ssize_t n = read(drivers_info[idx].resp_fd_read, buffer, sizeof(buffer)-1);
    if (n < 0) {
        perror("read response");
        return;
    }
    buffer[n] = '\0';
    if (n > 0 && buffer[n-1] == '\n') buffer[n-1] = '\0';
    printf("Driver %d: %s\n", pid, buffer);
}
// Читает задачу напрямую из разделяемой памяти
void get_status(pid_t pid) {
    // Ищем драйвер в разделяемой памяти
    int slot = find_driver_slot_by_pid(pid);
    if (slot == -1) {
        printf("Driver with PID %d not found\n", pid);
        return;
    }
    // Получаем указатель на статус драйвера
    struct driver_status *ds = &shm->drivers[slot];
    if (ds->busy) {
        printf("Driver %d: Busy %d\n", pid, ds->remaining);
    } else {
        printf("Driver %d: Available\n", pid);
    }
}

// Функция вывода статуса всех драйверов
void get_drivers() {
    if (shm->count == 0) {
        printf("No drivers\n");
        return;
    }
    for (int i = 0; i < shm->count; i++) {
        if (shm->drivers[i].active) {
            struct driver_status *ds = &shm->drivers[i];
            if (ds->busy) {
                printf("PID %d: Busy %d\n", ds->pid, ds->remaining);
            } else {
                printf("PID %d: Available\n", ds->pid);
            }
        }
    }
}

static volatile sig_atomic_t cli_running = 1;
void cli_signal_handler(int sig) {
    cli_running = 0;
}

int main() {
    // Удаление старой разделяемой памяти (если есть)
    shm_unlink(SHM_NAME);
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd < 0) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    if (ftruncate(shm_fd, sizeof(struct shared_mem)) < 0) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }
    // Отображаем разделяемую память в адресное пространство процесса
    shm = mmap(NULL, sizeof(struct shared_mem), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    memset(shm, 0, sizeof(struct shared_mem));

    signal(SIGINT, cli_signal_handler);
    signal(SIGTERM, cli_signal_handler);

    // Справка по командам
    printf("Taxi CLI started. Commands:\n");
    printf("  create_driver\n");
    printf("  send_task <pid> <task_timer>\n");
    printf("  get_status <pid>\n");
    printf("  get_drivers\n");
    printf("  exit\n");

    char input[256];
    while (cli_running) {
        printf("> ");
        fflush(stdout);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        char *cmd = strtok(input, " \t\n");
        if (cmd == NULL) continue;

        // Обработка команды create_driver
        if (strcmp(cmd, "create_driver") == 0) {
            create_driver();
        } 
        // Обработка команды send_task
        else if (strcmp(cmd, "send_task") == 0) {
            char *pid_str = strtok(NULL, " \t\n");
            char *timer_str = strtok(NULL, " \t\n");
            if (pid_str && timer_str) {
                pid_t pid = atoi(pid_str);
                int task_timer = atoi(timer_str);
                if (pid > 0 && task_timer > 0) {
                    send_task(pid, task_timer);
                } else {
                    printf("Invalid arguments\n");
                }
            } else {
                printf("Usage: send_task <pid> <task_timer>\n");
            }
        } 
        // Обработка команды get_status
        else if (strcmp(cmd, "get_status") == 0) {
            char *pid_str = strtok(NULL, " \t\n");
            if (pid_str) {
                pid_t pid = atoi(pid_str);
                if (pid > 0) {
                    get_status(pid);
                } else {
                    printf("Invalid pid\n");
                }
            } else {
                printf("Usage: get_status <pid>\n");
            }
        } 
        // Обработка команды get_drivers
        else if (strcmp(cmd, "get_drivers") == 0) {
            get_drivers();
        } 
        // Обработка команды exit
        else if (strcmp(cmd, "exit") == 0) {
            break;
        } 
        // Неизвестная команда
        else {
            printf("Unknown command\n");
        }
    }

    // Завершение всех драйверов
    for (int i = 0; i < num_drivers; i++) {
        kill(drivers_info[i].pid, SIGTERM);
        waitpid(drivers_info[i].pid, NULL, 0);
        close(drivers_info[i].cmd_fd_write);
        close(drivers_info[i].resp_fd_read);
    }

    // Очистка разделяемой памяти
    munmap(shm, sizeof(struct shared_mem));
    close(shm_fd);
    shm_unlink(SHM_NAME);

    printf("CLI terminated.\n");
    return 0;
}