#include <stdio.h>
#include <windows.h>

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

int main(void) {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    int choice;
    double a, b, result;
    do {
        printf("\n--- Kalkulyator ---\n");
        printf("1. Slozhenie\n");
        printf("2. Vychitanie\n");
        printf("3. Umnozhenie\n");
        printf("4. Delenie\n");
        printf("5. Vyhod\n");
        printf("Vyberite: ");
        scanf("%d", &choice);
        if (choice == 5) break;
        if (choice < 1 || choice > 4) {
            printf("Nevernyy vybor!\n");
            continue;
        }
        printf("Vvedite dva chisla: ");
        scanf("%lf %lf", &a, &b);
        switch (choice) {
            case 1: result = add(a, b); break;
            case 2: result = sub(a, b); break;
            case 3: result = mul(a, b); break;
            case 4: result = divi(a, b); break;
        }
        printf("Rezul'tat: %.2lf\n", result);
    } while (1);
    return 0;
}