#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <windows.h>

unsigned int ip_to_int(const char *ip) {
    unsigned int a, b, c, d;
    sscanf(ip, "%u.%u.%u.%u", &a, &b, &c, &d);
    return (a << 24) | (b << 16) | (c << 8) | d;
}

void int_to_ip(unsigned int ip, char *buf) {
    sprintf(buf, "%u.%u.%u.%u", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
            (ip >> 8) & 0xFF, ip & 0xFF);
}

void process_packets(const char *gateway_ip, const char *mask_ip, int N, int *same, int *total) {
    unsigned int gw = ip_to_int(gateway_ip);
    unsigned int mask = ip_to_int(mask_ip);
    unsigned int network = gw & mask;
    srand(time(NULL));
    int same_network = 0;
    for (int i = 0; i < N; i++) {
        unsigned int ip = (unsigned int)rand() | ((unsigned int)rand() << 16);
        if ((ip & mask) == network) same_network++;
    }
    *same = same_network;
    *total = N;
}

int main(int argc, char *argv[]) {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    if (argc != 4) {
        printf("Ispol'zovanie: %s <IP shlyuza> <maska> <N>\n", argv[0]);
        return 1;
    }
    int N = atoi(argv[3]);
    if (N <= 0) {
        printf("N dolzhno byt' > 0\n");
        return 1;
    }
    int same, total;
    process_packets(argv[1], argv[2], N, &same, &total);
    printf("Vsego: %d\n", total);
    printf("V svoey podseti: %d (%.2f%%)\n", same, (double)same / total * 100);
    printf("V drugih: %d (%.2f%%)\n", total - same, (double)(total - same) / total * 100);
    return 0;
}