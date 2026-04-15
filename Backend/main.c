#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define USERS_FILE "users.txt"

// ---------- REGISTER ----------
void registerUser(char *username, char *password) {
    char u[50], p[50], role[10];

    FILE *fp = fopen(USERS_FILE, "r");
    if (fp != NULL) {
        while (fscanf(fp, "%[^|]|%[^|]|%[^\n]%*c", u, p, role) == 3) {
            if (strcmp(username, u) == 0) {
                printf("EXISTS\n");
                fclose(fp);
                return;
            }
        }
        fclose(fp);
    }

    fp = fopen(USERS_FILE, "a");
    fprintf(fp, "%s|%s|user\n", username, password);
    fclose(fp);

    printf("OK\n");
}

// ---------- LOGIN ----------
void loginUser(char *username, char *password) {
    char u[50], p[50], role[10];

    if (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0) {
        printf("ADMIN\n");
        return;
    }

    FILE *fp = fopen(USERS_FILE, "r");
    if (fp != NULL) {
        while (fscanf(fp, "%[^|]|%[^|]|%[^\n]%*c", u, p, role) == 3) {
            if (strcmp(username, u) == 0 && strcmp(password, p) == 0) {
                printf("USER\n");
                fclose(fp);
                return;
            }
        }
        fclose(fp);
    }

    printf("INVALID\n");
}

// ---------- MAIN ----------
int main(int argc, char *argv[]) {

    // 🔥 WEB MODE
    if (argc >= 4) {
        if (strcmp(argv[1], "register") == 0)
            registerUser(argv[2], argv[3]);

        else if (strcmp(argv[1], "login") == 0)
            loginUser(argv[2], argv[3]);

        return 0;
    }

    // 🖥️ TERMINAL MODE
    int ch;
    char username[50], password[50];

    while (1) {
        printf("\n1.Register\n2.Login\n3.Exit\nChoice: ");
        scanf("%d", &ch);

        if (ch == 1) {
            printf("Username: ");
            scanf("%s", username);
            printf("Password: ");
            scanf("%s", password);
            registerUser(username, password);
        }
        else if (ch == 2) {
            printf("Username: ");
            scanf("%s", username);
            printf("Password: ");
            scanf("%s", password);
            loginUser(username, password);
        }
        else break;
    }
}