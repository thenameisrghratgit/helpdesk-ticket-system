#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FILE_NAME "ticketgen.txt"

char username[50];

// ---------- SERVICE CODE ----------
void getServiceCode(char *service, char code[]) {

    if (strstr(service, "Wi-Fi")) strcpy(code, "WIFI");
    else if (strstr(service, "Electrical")) strcpy(code, "ELEC");
    else if (strstr(service, "Air Conditioning")) strcpy(code, "AC");
    else if (strstr(service, "IT Support")) strcpy(code, "IT");
    else if (strstr(service, "Projector")) strcpy(code, "PROJ");
    else if (strstr(service, "Plumbing")) strcpy(code, "PLMB");
    else if (strstr(service, "Carpentry")) strcpy(code, "CARP");
    else if (strstr(service, "Cleaning")) strcpy(code, "CLN");
    else if (strstr(service, "Laboratory")) strcpy(code, "LAB");
    else if (strstr(service, "Printer")) strcpy(code, "PRNT");
    else if (strstr(service, "Access")) strcpy(code, "CARD");
    else if (strstr(service, "Network")) strcpy(code, "NET");
    else if (strstr(service, "Hostel")) strcpy(code, "HOST");
    else if (strstr(service, "Transport")) strcpy(code, "BUS");
    else if (strstr(service, "Library")) strcpy(code, "LIB");
    else if (strstr(service, "Software")) strcpy(code, "SW");
    else if (strstr(service, "Campus")) strcpy(code, "CMP");
    else strcpy(code, "OTHR");
}

// ---------- COUNT PER SERVICE ----------
int countServiceTickets(char *serviceCode) {
    FILE *fp = fopen(FILE_NAME, "r");
    int count = 0;
    char line[300], id[50];

    while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%[^|]", id);

        if (strstr(id, serviceCode))
            count++;
    }

    fclose(fp);
    return count;
}

// ---------- RAISE ----------
void raiseTicket(char *name, char *service, char *problem) {

    FILE *fp = fopen(FILE_NAME, "a");
    if (!fp) return;

    char code[10];
    getServiceCode(service, code);

    int count = countServiceTickets(code) + 1;

    char id[50];
    sprintf(id, "2026%s%02d", code, count);  // format

    fprintf(fp, "%s|%s|%s|%s|%s|Open\n",
            id, username, name, service, problem);

    fclose(fp);

    printf("OK|%s\n", id);
}

// ---------- CHECK ----------
void checkStatus(char *id) {
    FILE *fp = fopen(FILE_NAME, "r");
    if (!fp) return;

    char line[300];
    int found = 0;

    char my_status[20];
    int people_ahead = 0;

    while (fgets(line, sizeof(line), fp)) {

        char tid[50], u[50], name[50], serv[50], prob[100], status[20];

        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]",
               tid, u, name, serv, prob, status);

        if (strcmp(tid, id) == 0) {
            found = 1;
            strcpy(my_status, status);
        }

        if (strcmp(status, "Open") == 0 && strcmp(tid, id) != 0)
            people_ahead++;
    }

    fclose(fp);

    if (!found) {
        printf("NOTFOUND\n");
        return;
    }

    printf("FOUND|%s|%d\n", my_status, people_ahead);
}

// ---------- MAIN ----------
int main(int argc, char *argv[]) {

    // 🔥 WEB MODE
    if (argc >= 5) {
        strcpy(username, argv[1]);
        raiseTicket(argv[2], argv[3], argv[4]);
        return 0;
    }

    if (argc == 3) {
        checkStatus(argv[2]);
        return 0;
    }

    // TERMINAL MODE
    int ch;
    char name[50], service[50], problem[100];

    printf("Username: ");
    scanf("%s", username);

    while (1) {
        printf("\n1.Raise\n2.Check\n3.Exit\nChoice:");
        scanf("%d", &ch);

        if (ch == 1) {
            printf("Name: ");
            scanf(" %[^\n]", name);

            printf("Service: ");
            scanf(" %[^\n]", service);

            printf("Problem: ");
            scanf(" %[^\n]", problem);

            raiseTicket(name, service, problem);
        }
        else if (ch == 2) {
            char id[50];
            printf("Enter ID: ");
            scanf("%s", id);

            checkStatus(id);
        }
        else break;
    }
}