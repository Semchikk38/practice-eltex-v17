#ifndef CONTACT_H
#define CONTACT_H

#define MAX_STR 50
#define MAX_PHONES 3
#define MAX_EMAILS 2
#define MAX_SOCIAL 2
#define MAX_MESSENGERS 2

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
} Contact;

#endif