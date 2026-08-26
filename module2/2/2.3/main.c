#include <stdio.h>
#include <math.h>
#include <windows.h>

typedef double (*operation)(double, double);

double add(double a, double b) { return a + b; }
double sub(double a, double b) { return a - b; }
double mul(double a, double b) { return a * b; }
double divi(double a, double b) {
    if (b == 0) {
        printf("Oshibka: delenie na nol'!\n");
        return 0;
    }
    return a / b;
}
double power(double a, double b) { return pow(a, b); }

typedef struct {
    const char *name;
    operation func;
} Command;

Command commands[] = {
    {"Slozhenie", add},
    {"Vychitanie", sub},
    {"Umnozhenie", mul},
    {"Delenie", divi},
    {"Vozvedenie v stepen'", power}
};
int cmd_count = sizeof(commands) / sizeof(commands[0]);

int main(void) {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    int choice;
    double a, b;
    do {
        printf("\n--- Kalkulyator (dinamicheskie komandy) ---\n");
        for (int i = 0; i < cmd_count; i++) {
            printf("%d. %s\n", i + 1, commands[i].name);
        }
        printf("%d. Vyhod\n", cmd_count + 1);
        printf("Vyberite: ");
        scanf("%d", &choice);
        if (choice == cmd_count + 1) break;
        if (choice < 1 || choice > cmd_count) {
            printf("Nevernyy vybor!\n");
            continue;
        }
        printf("Vvedite dva chisla: ");
        scanf("%lf %lf", &a, &b);
        double result = commands[choice - 1].func(a, b);
        printf("Rezul'tat: %.2lf\n", result);
    } while (1);
    return 0;
}