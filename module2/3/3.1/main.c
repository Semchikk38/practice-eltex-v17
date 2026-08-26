#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <windows.h>

void mode_to_str(int mode, char *str) {
    str[0] = (mode & 4) ? 'r' : '-';
    str[1] = (mode & 2) ? 'w' : '-';
    str[2] = (mode & 1) ? 'x' : '-';
    str[3] = '\0';
}

int str_to_mode(const char *str) {
    int m = 0;
    if (str[0] == 'r') m |= 4;
    if (str[1] == 'w') m |= 2;
    if (str[2] == 'x') m |= 1;
    return m;
}

void print_permissions(int perm) {
    int user = (perm >> 6) & 7;
    int group = (perm >> 3) & 7;
    int other = perm & 7;
    char u[4], g[4], o[4];
    mode_to_str(user, u);
    mode_to_str(group, g);
    mode_to_str(other, o);
    printf("Bukvennoe:   %s%s%s\n", u, g, o);
    printf("Cifrovoe:    %o\n", perm);
    printf("Bitovoe:     ");
    for (int i = 8; i >= 0; i--) {
        printf("%d", (perm >> i) & 1);
        if (i % 3 == 0 && i != 0) printf(" ");
    }
    printf("\n");
}

int parse_perm_string(const char *s) {
    if (strlen(s) != 9) {
        printf("Oshibka: stroka dolzhna soderzhat' 9 simvolov.\n");
        return -1;
    }
    char user_str[4] = {s[0], s[1], s[2], '\0'};
    char group_str[4] = {s[3], s[4], s[5], '\0'};
    char other_str[4] = {s[6], s[7], s[8], '\0'};
    int user = str_to_mode(user_str);
    int group = str_to_mode(group_str);
    int other = str_to_mode(other_str);
    return (user << 6) | (group << 3) | other;
}

int modify_permissions(int current_perm, const char *cmd) {
    int new_perm = current_perm;
    char who, op;
    char perms[4] = {0};
    if (strlen(cmd) < 3) {
        printf("Nevernyy format komandy.\n");
        return current_perm;
    }
    who = cmd[0];
    if (who != 'u' && who != 'g' && who != 'o' && who != 'a') {
        printf("Nedopustimyy who.\n");
        return current_perm;
    }
    op = cmd[1];
    if (op != '+' && op != '-' && op != '=') {
        printf("Nedopustimyy operator.\n");
        return current_perm;
    }
    int idx = 0;
    for (size_t i = 2; i < strlen(cmd) && idx < 3; i++) {
        if (cmd[i] == 'r' || cmd[i] == 'w' || cmd[i] == 'x') {
            perms[idx++] = cmd[i];
        }
    }
    if (idx == 0) {
        printf("Ne ukazany razresheniya.\n");
        return current_perm;
    }
    perms[idx] = '\0';
    int mask = 0;
    for (int i = 0; i < strlen(perms); i++) {
        if (perms[i] == 'r') mask |= 4;
        else if (perms[i] == 'w') mask |= 2;
        else if (perms[i] == 'x') mask |= 1;
    }
    int u = (current_perm >> 6) & 7;
    int g = (current_perm >> 3) & 7;
    int o = current_perm & 7;
    int apply_u = (who == 'u' || who == 'a');
    int apply_g = (who == 'g' || who == 'a');
    int apply_o = (who == 'o' || who == 'a');
    if (apply_u) {
        if (op == '+') u |= mask;
        else if (op == '-') u &= ~mask;
        else if (op == '=') u = mask;
    }
    if (apply_g) {
        if (op == '+') g |= mask;
        else if (op == '-') g &= ~mask;
        else if (op == '=') g = mask;
    }
    if (apply_o) {
        if (op == '+') o |= mask;
        else if (op == '-') o &= ~mask;
        else if (op == '=') o = mask;
    }
    new_perm = (u << 6) | (g << 3) | o;
    return new_perm;
}

int main(void) {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    int choice;
    do {
        printf("\n--- Prava dostupa ---\n");
        printf("1. Vvesti prava (bukvenno ili cifrovo)\n");
        printf("2. Pokazat' prava fayla\n");
        printf("3. Modificirovat' prava (virtual'no)\n");
        printf("4. Vyhod\n");
        printf("Vyberite: ");
        scanf("%d", &choice);
        getchar();
        if (choice == 1) {
            char input[20];
            printf("Vvedite prava (naprimer, 755 ili rwxr-xr--): ");
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = '\0';
            int perm;
            if (input[0] >= '0' && input[0] <= '9') {
                perm = strtol(input, NULL, 8);
                if (perm < 0 || perm > 0777) {
                    printf("Nedopustimoe znachenie.\n");
                    continue;
                }
            } else {
                perm = parse_perm_string(input);
                if (perm == -1) continue;
            }
            print_permissions(perm);
        } else if (choice == 2) {
            char filename[256];
            printf("Vvedite imya fayla: ");
            fgets(filename, sizeof(filename), stdin);
            filename[strcspn(filename, "\n")] = '\0';
            struct stat st;
            if (stat(filename, &st) != 0) {
                perror("stat");
                continue;
            }
            int perm = st.st_mode & 0777;
            print_permissions(perm);
        } else if (choice == 3) {
            int current;
            printf("Vvedite tekushie prava (vosmerichno): ");
            scanf("%o", &current);
            getchar();
            if (current < 0 || current > 0777) {
                printf("Nedopustimoe znachenie.\n");
                continue;
            }
            printf("Tekushie prava:\n");
            print_permissions(current);
            char cmd[20];
            printf("Vvedite komandu (naprimer, u+x): ");
            fgets(cmd, sizeof(cmd), stdin);
            cmd[strcspn(cmd, "\n")] = '\0';
            int new_perm = modify_permissions(current, cmd);
            printf("Novye prava:\n");
            print_permissions(new_perm);
        } else if (choice == 4) {
            break;
        } else {
            printf("Nevernyy vybor.\n");
        }
    } while (1);
    return 0;
}