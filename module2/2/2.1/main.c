/*
 * 2.1. Telefonnaya kniga (massivy)
 * Hranit kontakty v staticheskom massive struktur.
 * Realizovano: dobavlenie, redaktirovanie, udalenie, prosmotr.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#define MAX_CONTACTS 100
#define MAX_STR 50
#define MAX_PHONES 3
#define MAX_EMAILS 2
#define MAX_SOCIAL 2
#define MAX_MESSENGERS 2

// Struktura kontakta
typedef struct {
    char surname[MAX_STR];
    char name[MAX_STR];
    char patronymic[MAX_STR];
    char job[MAX_STR];
    char position[MAX_STR];
    char phones[MAX_PHONES][MAX_STR];
    int phone_count;
    char emails[MAX_EMAILS][MAX_STR];
    int email_count;
    char social[MAX_SOCIAL][MAX_STR];
    int social_count;
    char messengers[MAX_MESSENGERS][MAX_STR];
    int messenger_count;
    int is_active;
} Contact;

Contact phonebook[MAX_CONTACTS];
int total_contacts = 0;

// Prototip funktsii testov (from test.c)
void run_tests(void);

// Funktsii
void clear_input_buffer(void) {
    while (getchar() != '\n');
}

void add_contact(void) {
    if (total_contacts >= MAX_CONTACTS) {
        printf("Telefonnaya kniga perepolnena!\n");
        return;
    }
    int idx = -1;
    for (int i = 0; i < MAX_CONTACTS; i++) {
        if (!phonebook[i].is_active) {
            idx = i;
            break;
        }
    }
    if (idx == -1) {
        printf("Net svobodnyh slotov!\n");
        return;
    }
    Contact *c = &phonebook[idx];
    printf("Vvedite familiyu: ");
    fgets(c->surname, MAX_STR, stdin);
    c->surname[strcspn(c->surname, "\n")] = '\0';
    printf("Vvedite imya: ");
    fgets(c->name, MAX_STR, stdin);
    c->name[strcspn(c->name, "\n")] = '\0';
    printf("Vvedite otchestvo (mozhno ostavit' pustym): ");
    fgets(c->patronymic, MAX_STR, stdin);
    c->patronymic[strcspn(c->patronymic, "\n")] = '\0';
    printf("Vvedite mesto raboty: ");
    fgets(c->job, MAX_STR, stdin);
    c->job[strcspn(c->job, "\n")] = '\0';
    printf("Vvedite dolzhnost': ");
    fgets(c->position, MAX_STR, stdin);
    c->position[strcspn(c->position, "\n")] = '\0';

    int count;
    printf("Skolko telefonov (maks. %d): ", MAX_PHONES);
    scanf("%d", &count);
    clear_input_buffer();
    if (count > MAX_PHONES) count = MAX_PHONES;
    c->phone_count = 0;
    for (int i = 0; i < count; i++) {
        printf("Telefon %d: ", i+1);
        fgets(c->phones[i], MAX_STR, stdin);
        c->phones[i][strcspn(c->phones[i], "\n")] = '\0';
        c->phone_count++;
    }
    
    c->is_active = 1;
    total_contacts++;
    printf("Kontakt dobavlen!\n");
}

int find_contact_by_name(const char *surname, const char *name) {
    for (int i = 0; i < MAX_CONTACTS; i++) {
        if (phonebook[i].is_active &&
            strcmp(phonebook[i].surname, surname) == 0 &&
            strcmp(phonebook[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

void edit_contact(void) {
    char surname[MAX_STR], name[MAX_STR];
    printf("Vvedite familiyu: ");
    fgets(surname, MAX_STR, stdin);
    surname[strcspn(surname, "\n")] = '\0';
    printf("Vvedite imya: ");
    fgets(name, MAX_STR, stdin);
    name[strcspn(name, "\n")] = '\0';
    int idx = find_contact_by_name(surname, name);
    if (idx == -1) {
        printf("Kontakt ne nayden!\n");
        return;
    }
    Contact *c = &phonebook[idx];
    char buffer[MAX_STR];
    printf("Novaya familiya (%s): ", c->surname);
    fgets(buffer, MAX_STR, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
    if (strlen(buffer) > 0) strcpy(c->surname, buffer);
    printf("Kontakt obnovlen.\n");
}

void delete_contact(void) {
    char surname[MAX_STR], name[MAX_STR];
    printf("Vvedite familiyu: ");
    fgets(surname, MAX_STR, stdin);
    surname[strcspn(surname, "\n")] = '\0';
    printf("Vvedite imya: ");
    fgets(name, MAX_STR, stdin);
    name[strcspn(name, "\n")] = '\0';
    int idx = find_contact_by_name(surname, name);
    if (idx == -1) {
        printf("Kontakt ne nayden!\n");
        return;
    }
    phonebook[idx].is_active = 0;
    total_contacts--;
    printf("Kontakt udalen.\n");
}

void list_contacts(void) {
    printf("\n--- Spisok kontaktov ---\n");
    int found = 0;
    for (int i = 0; i < MAX_CONTACTS; i++) {
        if (phonebook[i].is_active) {
            found = 1;
            Contact *c = &phonebook[i];
            printf("%s %s %s\n", c->surname, c->name, c->patronymic);
            printf("  Rabota: %s, dolzhnost': %s\n", c->job, c->position);
            printf("  Telefony: ");
            for (int j = 0; j < c->phone_count; j++) printf("%s ", c->phones[j]);
            printf("\n");
        }
    }
    if (!found) printf("Net kontaktov.\n");
}

int main(int argc, char *argv[]) {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    
    if (argc == 2 && strcmp(argv[1], "--test") == 0) {
        run_tests();
        return 0;
    }
    
    for (int i = 0; i < MAX_CONTACTS; i++) phonebook[i].is_active = 0;
    
    int choice;
    do {
        printf("\n--- Telefonnaya kniga ---\n");
        printf("1. Dobavit' kontakt\n");
        printf("2. Redaktirovat' kontakt\n");
        printf("3. Udalit' kontakt\n");
        printf("4. Pokazat' vse kontakty\n");
        printf("5. Vyhod\n");
        printf("Vash vybor: ");
        scanf("%d", &choice);
        clear_input_buffer();
        switch (choice) {
            case 1: add_contact(); break;
            case 2: edit_contact(); break;
            case 3: delete_contact(); break;
            case 4: list_contacts(); break;
            case 5: printf("Do svidaniya!\n"); break;
            default: printf("Nevernyy vybor!\n");
        }
    } while (choice != 5);
    return 0;
}