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

char username[50];

// ── QUEUE NODE ───────────────────────────────────────────────────────
struct Ticket
{
    char id[50];
    struct Ticket *next;
};

// ── BST NODE (one per service) ───────────────────────────────────────
struct ServiceNode
{
    int serviceNum;
    struct Ticket *front;   // queue front
    struct Ticket *rear;    // queue rear
    struct ServiceNode *left;
    struct ServiceNode *right;
};

struct ServiceNode *bstRoot = NULL;

// ── SERVICE STRING → NUMBER ──────────────────────────────────────────
int getServiceNum(char *service)
{
    if (strstr(service, "Wi-Fi"))          return WIFI;
    if (strstr(service, "Electrical"))     return ELECTRICAL;
    if (strstr(service, "Air Conditioning"))return AC;
    if (strstr(service, "IT Support"))     return IT;
    if (strstr(service, "Projector"))      return PROJECTOR;
    if (strstr(service, "Plumbing"))       return PLUMBING;
    if (strstr(service, "Carpentry"))      return CARPENTRY;
    if (strstr(service, "Cleaning"))       return CLEANING;
    if (strstr(service, "Laboratory"))     return LAB;
    if (strstr(service, "Printer"))        return PRINTER;
    if (strstr(service, "Access"))         return ACCESS;
    if (strstr(service, "Network"))        return NETWORK;
    if (strstr(service, "Hostel"))         return HOSTEL;
    if (strstr(service, "Transport"))      return TRANSPORT;
    if (strstr(service, "Library"))        return LIBRARY;
    if (strstr(service, "Software"))       return SOFTWARE;
    if (strstr(service, "Campus"))         return CAMPUS;
    return OTHER;
}

// ── SERVICE NUMBER → SHORT CODE (for ticket ID) ──────────────────────
void getServiceCode(int num, char code[])
{
    const char *codes[] = {"","WIFI","ELEC","AC","IT","PROJ","PLMB",
                            "CARP","CLN","LAB","PRNT","CARD","NET",
                            "HOST","BUS","LIB","SW","CMP","OTHR"};
    if (num >= 1 && num <= 18)
        strcpy(code, codes[num]);
    else
        strcpy(code, "OTHR");
}

// ── BST: find or create service node ────────────────────────────────
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

// ── QUEUE OPS on a service node ──────────────────────────────────────
void enqueueToService(struct ServiceNode *sn, char *ticketID)
{
    struct Ticket *t = malloc(sizeof(struct Ticket));
    strcpy(t->id, ticketID);
    t->next = NULL;
    if (sn->front == NULL) sn->front = sn->rear = t;
    else { sn->rear->next = t; sn->rear = t; }
}

// ── COUNT tickets per service code in file ───────────────────────────
int countServiceTickets(char *serviceCode)
{
    FILE *fp = fopen(FILE_NAME, "r");
    int count = 0;
    char line[300], id[50];
    while (fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%[^|]", id);
        if (strstr(id, serviceCode)) count++;
    }
    fclose(fp);
    return count;
}

// ── RAISE TICKET ─────────────────────────────────────────────────────
void raiseTicket(char *name, char *service, char *problem)
{
    FILE *fp = fopen(FILE_NAME, "a");
    if (!fp) return;

    int snum = getServiceNum(service);
    char code[10];
    getServiceCode(snum, code);

    int count = countServiceTickets(code) + 1;

    char id[50];
    sprintf(id, "2026%s%02d", code, count);

    fprintf(fp, "%s|%s|%s|%s|%s|Open\n", id, username, name, service, problem);
    fclose(fp);

    // insert into BST + enqueue
    bstRoot = bstInsert(bstRoot, snum);
    struct ServiceNode *sn = bstSearch(bstRoot, snum);
    enqueueToService(sn, id);

    printf("OK|%s\n", id);
}

// ── CHECK STATUS: position within that service queue ─────────────────
void checkStatus(char *id)
{
    FILE *fp = fopen(FILE_NAME, "r");
    if (!fp) { printf("NOTFOUND\n"); return; }

    // find service of this ticket
    char line[300], my_status[20] = "";
    char found_service[50] = "";
    int found = 0;

    while (fgets(line, sizeof(line), fp))
    {
        char tid[50], u[50], name[50], serv[50], prob[100], status[20];
        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]",
               tid, u, name, serv, prob, status);
        if (strcmp(tid, id) == 0)
        {
            strcpy(my_status, status);
            strcpy(found_service, serv);
            found = 1;
            break;
        }
    }
    fclose(fp);

    if (!found) { printf("NOTFOUND\n"); return; }

    // count position in that service's queue
    int snum = getServiceNum(found_service);
    struct ServiceNode *sn = bstSearch(bstRoot, snum);
    int pos = 0;
    if (sn)
    {
        struct Ticket *curr = sn->front;
        while (curr)
        {
            if (strcmp(curr->id, id) == 0) break;
            pos++;
            curr = curr->next;
        }
    }

    printf("FOUND|%s|%d\n", my_status, pos);
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

// ── MAIN ─────────────────────────────────────────────────────────────
int main(int argc, char *argv[])
{
    loadQueue();

    if (argc >= 5)
    {
        strcpy(username, argv[1]);
        raiseTicket(argv[2], argv[3], argv[4]);
        return 0;
    }

    if (argc == 3)
    {
        checkStatus(argv[2]);
        return 0;
    }

    int ch;
    char name[50], service[50], problem[100];

    printf("Username: ");
    scanf("%s", username);

    while (1)
    {
        printf("\n1.Raise\n2.Check\n3.Exit\nChoice: ");
        scanf("%d", &ch);

        if (ch == 1)
        {
            printf("Name: ");
            scanf(" %[^\n]", name);
            printf("Service: ");
            scanf(" %[^\n]", service);
            printf("Problem: ");
            scanf(" %[^\n]", problem);
            raiseTicket(name, service, problem);
        }
        else if (ch == 2)
        {
            char id[50];
            printf("Enter ID: ");
            scanf("%s", id);
            checkStatus(id);
        }
        else break;
    }
}
