#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "list.h"
#include "contact.h"

// Prototip funktsii testov
void run_tests(void);

int main(int argc, char *argv[]) {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    
    // Proverka argumenta
    if (argc == 2 && strcmp(argv[1], "--test") == 0) {
        run_tests();
        return 0;
    }
    
    printf("Zapustite ./main --test dlya avtomaticheskoy proverki.\n");
    return 0;
}