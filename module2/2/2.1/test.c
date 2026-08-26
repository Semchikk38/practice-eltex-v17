#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#define MAX_CONTACTS 100
#define MAX_STR 50

typedef struct {
    char surname[MAX_STR];
    char name[MAX_STR];
    char patronymic[MAX_STR];
    char job[MAX_STR];
    char position[MAX_STR];
    char phones[3][MAX_STR];
    int phone_count;
    char emails[2][MAX_STR];
    int email_count;
    char social[2][MAX_STR];
    int social_count;
    char messengers[2][MAX_STR];
    int messenger_count;
    int is_active;
} Contact;

extern Contact phonebook[MAX_CONTACTS];
extern int total_contacts;
void list_contacts(void);
int find_contact_by_name(const char *surname, const char *name);
void clear_input_buffer(void);

void run_tests(void) {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    
    printf("\n=== AVTOMATICHESKIY TEST 2.1 ===\n");

    for (int i = 0; i < MAX_CONTACTS; i++) phonebook[i].is_active = 0;
    total_contacts = 0;

    Contact c1 = {"Ivanov","Ivan","Ivanovich","Inzhener","Starshiy",{{"+7-999-111-22-33"}},1,{""},0,{""},0,{""},0,1};
    Contact c2 = {"Petrov","Petr","Petrovich","IT","Razrabotchik",{{"+7-999-222-33-44"}},1,{""},0,{""},0,{""},0,1};
    Contact c3 = {"Sidorov","Sidor","Sidorovich","Buhgalteriya","Glavnyy",{{"+7-999-333-44-55"}},1,{""},0,{""},0,{""},0,1};
    phonebook[0] = c1;
    phonebook[1] = c2;
    phonebook[2] = c3;
    total_contacts = 3;

    printf("\n1. Spisok posle dobavleniya treh kontaktov:\n");
    list_contacts();

    int idx = find_contact_by_name("Petrov", "Petr");
    if (idx != -1) {
        strcpy(phonebook[idx].position, "Vedushiy razrabotchik");
        printf("\n2. Dolzhnost' Petrova izmenena na 'Vedushiy razrabotchik'.\n");
    } else {
        printf("Oshibka: Petrov ne nayden\n");
    }

    printf("\n3. Spisok posle redaktirovaniya:\n");
    list_contacts();

    idx = find_contact_by_name("Sidorov", "Sidor");
    if (idx != -1) {
        phonebook[idx].is_active = 0;
        total_contacts--;
        printf("\n4. Kontakt Sidorov udalen.\n");
    } else {
        printf("Oshibka: Sidorov ne nayden\n");
    }

    printf("\n5. Finalnyy spisok (dolzhno ostat'sya 2 kontakta):\n");
    list_contacts();

    int active = 0;
    for (int i = 0; i < MAX_CONTACTS; i++) {
        if (phonebook[i].is_active) active++;
    }
    if (active == 2) {
        printf("\nTest proyden! Ostalos' 2 kontakta.\n");
    } else {
        printf("\nTest provalen! Ozhidalos' 2 kontakta, polucheno %d.\n", active);
    }
    printf("=== TEST ZAVERSHEN ===\n");
}