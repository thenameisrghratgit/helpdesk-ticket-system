#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FILE_NAME "ticketgen.txt"

// ── SERVICE NUMBER DEFINES ──────────────────────────────────────────
#define WIFI        1
#define ELECTRICAL  2
#define AC          3
#define IT          4
#define PROJECTOR   5
#define PLUMBING    6
#define CARPENTRY   7
#define CLEANING    8
#define LAB         9
#define PRINTER     10
#define ACCESS      11
#define NETWORK     12
#define HOSTEL      13
#define TRANSPORT   14
#define LIBRARY     15
#define SOFTWARE    16
#define CAMPUS      17
#define OTHER       18

// ── QUEUE NODE ───────────────────────────────────────────────────────
struct Ticket
{
    char id[50];
    struct Ticket *next;
};

// ── BST NODE ─────────────────────────────────────────────────────────
struct ServiceNode
{
    int serviceNum;
    struct Ticket *front;
    struct Ticket *rear;
    struct ServiceNode *left;
    struct ServiceNode *right;
};

struct ServiceNode *bstRoot = NULL;

// ── SERVICE STRING → NUMBER ──────────────────────────────────────────
int getServiceNum(char *service)
{
    if (strstr(service, "Wi-Fi"))           return WIFI;
    if (strstr(service, "Electrical"))      return ELECTRICAL;
    if (strstr(service, "Air Conditioning"))return AC;
    if (strstr(service, "IT Support"))      return IT;
    if (strstr(service, "Projector"))       return PROJECTOR;
    if (strstr(service, "Plumbing"))        return PLUMBING;
    if (strstr(service, "Carpentry"))       return CARPENTRY;
    if (strstr(service, "Cleaning"))        return CLEANING;
    if (strstr(service, "Laboratory"))      return LAB;
    if (strstr(service, "Printer"))         return PRINTER;
    if (strstr(service, "Access"))          return ACCESS;
    if (strstr(service, "Network"))         return NETWORK;
    if (strstr(service, "Hostel"))          return HOSTEL;
    if (strstr(service, "Transport"))       return TRANSPORT;
    if (strstr(service, "Library"))         return LIBRARY;
    if (strstr(service, "Software"))        return SOFTWARE;
    if (strstr(service, "Campus"))          return CAMPUS;
    return OTHER;
}

// ── BST OPS ──────────────────────────────────────────────────────────
struct ServiceNode *bstInsert(struct ServiceNode *root, int snum)
{
    if (root == NULL)
    {
        struct ServiceNode *n = malloc(sizeof(struct ServiceNode));
        n->serviceNum = snum;
        n->front = n->rear = NULL;
        n->left = n->right = NULL;
        return n;
    }
    if (snum < root->serviceNum)
        root->left  = bstInsert(root->left,  snum);
    else if (snum > root->serviceNum)
        root->right = bstInsert(root->right, snum);
    return root;
}

struct ServiceNode *bstSearch(struct ServiceNode *root, int snum)
{
    if (root == NULL || root->serviceNum == snum) return root;
    if (snum < root->serviceNum) return bstSearch(root->left,  snum);
    return bstSearch(root->right, snum);
}

void enqueueToService(struct ServiceNode *sn, char *ticketID)
{
    struct Ticket *t = malloc(sizeof(struct Ticket));
    strcpy(t->id, ticketID);
    t->next = NULL;
    if (sn->front == NULL) sn->front = sn->rear = t;
    else { sn->rear->next = t; sn->rear = t; }
}

void dequeueFromService(struct ServiceNode *sn, char *id)
{
    if (sn == NULL || sn->front == NULL) return;
    if (strcmp(sn->front->id, id) == 0)
    {
        struct Ticket *tmp = sn->front;
        sn->front = sn->front->next;
        if (sn->front == NULL) sn->rear = NULL;
        free(tmp);
    }
}

// ── LOAD FILE → BST ──────────────────────────────────────────────────
void loadQueue()
{
    FILE *fp = fopen(FILE_NAME, "r");
    if (!fp) return;

    char line[300];
    while (fgets(line, sizeof(line), fp))
    {
        char tid[50], u[50], name[50], serv[50], prob[100], status[20];
        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]",
               tid, u, name, serv, prob, status);

        if (strcmp(status, "Open") == 0)
        {
            int snum = getServiceNum(serv);
            bstRoot = bstInsert(bstRoot, snum);
            struct ServiceNode *sn = bstSearch(bstRoot, snum);
            enqueueToService(sn, tid);
        }
    }
    fclose(fp);
}

// ── UPDATE TICKET STATUS ─────────────────────────────────────────────
void updateTicket(char *id, char *newStatus)
{
    FILE *fp  = fopen(FILE_NAME, "r");
    FILE *tmp = fopen("temp.txt", "w");

    char line[300];
    char found_service[50] = "";

    while (fgets(line, sizeof(line), fp))
    {
        char tid[50], u[50], name[50], serv[50], prob[100], status[20];
        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]",
               tid, u, name, serv, prob, status);

        if (strcmp(tid, id) == 0)
        {
            strcpy(status, newStatus);
            strcpy(found_service, serv);
        }

        fprintf(tmp, "%s|%s|%s|%s|%s|%s\n", tid, u, name, serv, prob, status);
    }

    fclose(fp);
    fclose(tmp);

    remove(FILE_NAME);
    rename("temp.txt", FILE_NAME);

    // dequeue from correct service queue when closed
    if (strcmp(newStatus, "Closed") == 0 && strlen(found_service) > 0)
    {
        int snum = getServiceNum(found_service);
        struct ServiceNode *sn = bstSearch(bstRoot, snum);
        dequeueFromService(sn, id);
    }

    printf("OK\n");
}

// ── MAIN ─────────────────────────────────────────────────────────────
int main(int argc, char *argv[])
{
    loadQueue();

    if (argc >= 3)
    {
        updateTicket(argv[1], argv[2]);
        return 0;
    }

    char id[50];
    printf("Enter ID: ");
    scanf("%s", id);

    updateTicket(id, "Closed");
}
