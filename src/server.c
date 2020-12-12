#include "import/server_static.h"

extern int errno;
#define MAX_THREAD 200

struct thData {
    int idThread;
    int sdCl;

    int user_id;    
};

typedef struct thData thData;

static void *treat(void *);
void raspunde(void *);

sqlite3 *db;

thData *tdDat;
int i = 0;

int main(int argc, char *argv[]) {

    db = openDatabase(DATABASE);

    if (argc == 2) {
        initializeDatabase(db);
    }

    struct sockaddr_in server;
    struct sockaddr_in from;
    int nr;
    int sd;
    int pid;
    
    pthread_t th[MAX_THREAD];
    tdDat = (thData*)malloc((sizeof(thData) * MAX_THREAD));


    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket() error\n");
        return errno;
    }

    int on = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(GATEWAY_IP);

    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
        perror("[server] bind() error\n");
        return errno;
    }

    if (listen(sd, 2) == -1) {
        perror("[server] listen() error\n");
        return errno;
    }

    printf("Waiting to port %d...\n", PORT);
    

    for(;;) {
        int client;

        int length = sizeof(from);

        if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0) {
            perror("[server] accept() error\n");
            continue;
        }

        tdDat[i].idThread = i;
        tdDat[i].sdCl = client;

        pthread_create(&th[i], NULL, &treat, tdDat + i);
    
        i++;
    }

    closeDatabase(db);
    return 0;
}

static void *treat(void *arg) {
    thData *tdL;
    tdL = (thData *)arg;

    printf("[thread] - %d - Waiting for the request...\n", tdL->idThread);

    pthread_detach(pthread_self());

    raspunde((thData *)arg);

    close((intptr_t)arg);
    return (NULL);
};

void raspunde(void *arg) {
    thData *tdL;
    tdL = (struct thData *)arg;

    int sd = tdL->sdCl;

    int REQUEST_TYPE;

    repeat:

    if (read(sd, &REQUEST_TYPE, sizeof(int)) <= 0) {
        printf("[Thread %d] ", tdL->idThread);
        perror("read() error\n");
    }

    User u;

    switch (REQUEST_TYPE) {
    case LOGIN: ;
        printf("Login Request\n");

        if (read(sd, &u, sizeof(User)) == -1) {
            perror("[server] " READ_ERROR);
            return;
        }

        verifyUser(db, &u);

        tdL->user_id = u.userID;

        if (write(sd, &u, sizeof(User)) == -1) {
            perror("[server] " WRITE_ERROR);
            return;
        }

        break;
    case SIGNUP: ;
        printf("Signup Request\n");

        if (read(sd, &u, sizeof(User)) == -1) {
            perror("[server] " READ_ERROR);
            return;
        }

        addUser(db, &u);

        if (write(sd, &u, sizeof(User)) == -1) {
            perror("[server] " WRITE_ERROR);
            return;
        }

        break;
    case LOGOUT: ;
        printf("User %d disconected\n", tdL->user_id);
        tdL->user_id = -1;
        break;

    default:
        break;
    }

    printf("Request end\n\n");

    goto repeat;
}
