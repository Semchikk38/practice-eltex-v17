#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

// Prototip funktsii iz main.c
void process_packets(const char *gateway_ip, const char *mask_ip, int N, int *same, int *total);

void run_tests(void) {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    printf("\n=== TEST 3.2 ===\n");
    int same, total;
    
    // Test 1: maska /24, shlyuz 192.168.1.1, N=100
    process_packets("192.168.1.1", "255.255.255.0", 100, &same, &total);
    if (total == 100 && same >= 0 && same <= 100) {
        printf("Test 1 proyden: total=%d, same=%d (v predelah normy)\n", total, same);
    } else {
        printf("Test 1 provalen: total=%d, same=%d\n", total, same);
    }
    
    // Test 2: maska /16, shlyuz 10.0.0.1, N=50
    process_packets("10.0.0.1", "255.255.0.0", 50, &same, &total);
    if (total == 50 && same >= 0 && same <= 50) {
        printf("Test 2 proyden: total=%d, same=%d\n", total, same);
    } else {
        printf("Test 2 provalen: total=%d, same=%d\n", total, same);
    }
    
    printf("=== TEST ZAVERSHEN ===\n");
}