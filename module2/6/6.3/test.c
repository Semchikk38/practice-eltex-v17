#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

void run_tests(void) {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    printf("\n=== TEST 6.3 ===\n");
    
    HMODULE handle = LoadLibrary("./plugins/add.dll");
    if (!handle) {
        handle = LoadLibrary("./plugins/add.so");
        if (!handle) {
            printf("Ne udalos' zagruzit' add.dll ili add.so\n");
            printf("Oshibka: %ld\n", GetLastError());
            return;
        }
    }
    
    double (*func)(double, double) = (double(*)(double, double))GetProcAddress(handle, "operation");
    if (!func) {
        printf("Ne naydena funktsiya operation v add.dll\n");
        FreeLibrary(handle);
        return;
    }
    
    double result = func(2.0, 3.0);
    if (result == 5.0) {
        printf("Test slozheniya proyden: 2+3=5\n");
    } else {
        printf("Oshibka: 2+3 = %.2f\n", result);
    }
    
    FreeLibrary(handle);
    printf("=== TEST ZAVERSHEN ===\n");
}