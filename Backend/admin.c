#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FILE_NAME "ticketgen.txt"

// ---------- UPDATE ----------
void updateTicket(int id, char *newStatus) {

    FILE *fp = fopen(FILE_NAME, "r");
    FILE *tmp = fopen("temp.txt", "w");

    char line[300];

    while (fgets(line, sizeof(line), fp)) {
        int tid;
        char u[50], name[50], serv[50], prob[100], status[20];

        sscanf(line, "%d|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]",
               &tid, u, name, serv, prob, status);

        if (tid == id)
            strcpy(status, newStatus);

        fprintf(tmp, "%d|%s|%s|%s|%s|%s\n",
                tid, u, name, serv, prob, status);
    }

    fclose(fp);
    fclose(tmp);

    remove(FILE_NAME);
    rename("temp.txt", FILE_NAME);

    printf("OK\n");
}

// ---------- MAIN ----------
int main(int argc, char *argv[]) {

    // 🔥 WEB MODE
    if (argc >= 3) {
        int id = atoi(argv[1]);
        updateTicket(id, argv[2]);
        return 0;
    }

    // 🖥️ TERMINAL MODE
    int id;
    printf("Enter ID: ");
    scanf("%d", &id);

    updateTicket(id, "Closed");
}