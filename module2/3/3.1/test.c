#include <stdio.h>
#include <string.h>
#include <windows.h>

// Prototipy funktsiy iz main.c
int parse_perm_string(const char *s);
int modify_permissions(int current_perm, const char *cmd);
void print_permissions(int perm);

void run_tests(void) {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    printf("\n=== TEST 3.1 ===\n");

    struct { const char *input; int expected; } parse_tests[] = {
        {"rwxr-xr--", 0754},
        {"--x--x--x", 0111},
        {"r--r--r--", 0444},
        {"rw-------", 0600}
    };
    int passed = 0;
    for (int i = 0; i < 4; i++) {
        int result = parse_perm_string(parse_tests[i].input);
        if (result == parse_tests[i].expected) {
            printf("Parsing '%s' -> %o OK\n", parse_tests[i].input, result);
            passed++;
        } else {
            printf("Parsing '%s' -> %o (ozhidalos' %o) FAIL\n",
                   parse_tests[i].input, result, parse_tests[i].expected);
        }
    }

    int base = 0755;
    struct { const char *cmd; int expected; } mod_tests[] = {
        {"u+x", 0755},
        {"g+w", 0775},
        {"o-r", 0754},
        {"a-x", 0644}
    };
    int cur = base;
    for (int i = 0; i < 4; i++) {
        int newp = modify_permissions(cur, mod_tests[i].cmd);
        if (newp == mod_tests[i].expected) {
            printf("Modifikatsiya '%s' iz %o -> %o OK\n", mod_tests[i].cmd, cur, newp);
            passed++;
        } else {
            printf("Modifikatsiya '%s' iz %o -> %o (ozhidalos' %o) FAIL\n",
                   mod_tests[i].cmd, cur, newp, mod_tests[i].expected);
        }
        cur = newp;
    }
    printf("Vsego proydeno: %d iz 8\n", passed);
    printf("=== TEST ZAVERSHEN ===\n");
}