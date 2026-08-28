#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define BUF_SIZE 1024

void child_work(int fd_read, int fd_write) {
    char ready_msg[] = "READY\n";
    write(fd_write, ready_msg, strlen(ready_msg));

    char header[512];
    while (1) {
        int i = 0;
        char c;
        while (read(fd_read, &c, 1) == 1 && c != '\n' && i < 510) header[i++] = c;
        header[i] = '\0';

        if (strcmp(header, "END") == 0) break;

        char fname[256];
        long fsize;
        sscanf(header, "%s %ld", fname, &fsize);

        char outname[300];
        snprintf(outname, sizeof(outname), "%s.copy", fname);
        int fd_out = open(outname, O_WRONLY | O_CREAT | O_TRUNC, 0644);

        long remaining = fsize;
        char buf[BUF_SIZE];
        while (remaining > 0) {
            ssize_t to_read = remaining > BUF_SIZE ? BUF_SIZE : remaining;
            ssize_t n = read(fd_read, buf, to_read);
            if (n <= 0) break;
            write(fd_out, buf, n);
            remaining -= n;
        }
        close(fd_out);
    }
}

void parent_work(int fd_read, int fd_write, char **files, int count) {
    char buf[BUF_SIZE];
    read(fd_read, buf, sizeof(buf)); // Ждем READY

    for (int i = 0; i < count; i++) {
        int fd_in = open(files[i], O_RDONLY);
        if (fd_in < 0) {
            fprintf(stderr, "Ошибка открытия %s: %s\n", files[i], strerror(errno));
            continue;
        }
        struct stat st;
        fstat(fd_in, &st);

        char header[512];
        int hlen = snprintf(header, sizeof(header), "%s %ld\n", files[i], (long)st.st_size);
        write(fd_write, header, hlen);

        ssize_t n;
        while ((n = read(fd_in, buf, BUF_SIZE)) > 0) {
            write(fd_write, buf, n);
        }
        close(fd_in);
    }
    const char *end = "END\n";
    write(fd_write, end, strlen(end));
}

int main(int argc, char *argv[]) {
    char *fifo_name = NULL;
    int file_start = 1;
    if (argc > 2 && strcmp(argv[1], "-p") == 0) {
        fifo_name = argv[2];
        file_start = 3;
    }

    int fd_read, fd_write, fd_read_c, fd_write_c;

    if (fifo_name) {
        mkfifo(fifo_name, 0666);
        int fd = open(fifo_name, O_RDWR); 
        fd_read = fd_write = fd_read_c = fd_write_c = fd;
    } else {
        int p2c[2], c2p[2];
        pipe(p2c); pipe(c2p);
        fd_write = p2c[1]; fd_read_c = p2c[0];
        fd_write_c = c2p[1]; fd_read = c2p[0];
    }

    pid_t pid = fork();
    if (pid == 0) {
        if (!fifo_name) { close(fd_write); close(fd_read); }
        child_work(fd_read_c, fd_write_c);
        exit(0);
    } else {
        if (!fifo_name) { close(fd_write_c); close(fd_read_c); }
        parent_work(fd_read, fd_write, argv + file_start, argc - file_start);
        wait(NULL);
        if (fifo_name) {
            close(fd_read);
            unlink(fifo_name);
        }
    }
    return 0;
}