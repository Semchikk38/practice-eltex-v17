#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <dirent.h>
#include <sys/stat.h>

#define PLUGIN_DIR "./plugins"

typedef double (*operation_func)(double, double);

typedef struct {
    char name[50];
    operation_func func;
    HMODULE handle;
} Command;

Command commands[100];
int cmd_count = 0;

void load_plugin(const char *path) {
    HMODULE handle = LoadLibrary(path);
    if (!handle) {
        printf("Oshibka zagruzki %s: error %ld\n", path, GetLastError());
        return;
    }
    
    operation_func func = (operation_func)GetProcAddress(handle, "operation");
    if (!func) {
        printf("V %s net funktsii operation\n", path);
        FreeLibrary(handle);
        return;
    }
    
    const char *filename = strrchr(path, '/');
    if (filename) filename++;
    else filename = path;
    
    char name[50];
    strcpy(name, filename);
    char *dot = strrchr(name, '.');
    if (dot) *dot = '\0';
    
    strcpy(commands[cmd_count].name, name);
    commands[cmd_count].func = func;
    commands[cmd_count].handle = handle;
    cmd_count++;
    printf("Zagruzhena operatsiya '%s'\n", name);
}

void load_plugins(void) {
    DIR *dir = opendir(PLUGIN_DIR);
    if (!dir) {
        mkdir(PLUGIN_DIR);
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        char path[256];
        snprintf(path, sizeof(path), "%s/%s", PLUGIN_DIR, entry->d_name);
        
        struct stat st;
        if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) {
            const char *ext = strrchr(entry->d_name, '.');
            if (ext && (strcmp(ext, ".dll") == 0 || strcmp(ext, ".so") == 0)) {
                load_plugin(path);
            }
        }
    }
    closedir(dir);
}

int main(void) {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    load_plugins();
    if (cmd_count == 0) {
        printf("Net zagruzhennyh operatsiy.\n");
        printf("Sozdayte .dll fayly v papke %s\n", PLUGIN_DIR);
        return 1;
    }
    
    int choice;
    double a, b;
    do {
        printf("\n--- Kalkulyator s pluginami ---\n");
        for (int i = 0; i < cmd_count; i++) {
            printf("%d. %s\n", i + 1, commands[i].name);
        }
        printf("%d. Vyhod\n", cmd_count + 1);
        printf("Vyberite: ");
        scanf("%d", &choice);
        if (choice == cmd_count + 1) break;
        if (choice < 1 || choice > cmd_count) {
            printf("Nevernyy vybor\n");
            continue;
        }
        printf("Vvedite dva chisla: ");
        scanf("%lf %lf", &a, &b);
        double result = commands[choice - 1].func(a, b);
        printf("Rezul'tat: %.2lf\n", result);
    } while (1);
    
    for (int i = 0; i < cmd_count; i++) {
        FreeLibrary(commands[i].handle);
    }
    
    return 0;
}