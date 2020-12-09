#include "import/server_static.h"

extern int errno;

typedef struct thData {
    int idThread;
    int cl;    
} thData;

static void *treat(void *);
void raspunde(void *);

sqlite3 *db;

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
    pthread_t th[100];
    int i = 0;

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
        thData *td;
        int length = sizeof(from);


        if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0) {
            perror("[server] accept() error\n");
            continue;
        }

        td = (struct thData *)malloc(sizeof(struct thData));
        td->idThread = i++;
        td->cl = client;

        pthread_create(&th[i], NULL, &treat, td);
    }

    closeDatabase(db);
    return 0;
}

static void *treat(void *arg) {
    struct thData tdL;
    tdL = *((struct thData *)arg);

    printf("[thread] - %d - Waiting for the request...\n", tdL.idThread);

    pthread_detach(pthread_self());

    raspunde((struct thData *)arg);

    close((intptr_t)arg);
    return (NULL);
};

void raspunde(void *arg) {
    struct thData tdL;
    tdL = *((struct thData *)arg);

    int sd = tdL.cl;

    int REQUEST_TYPE;

    if (read(sd, &REQUEST_TYPE, sizeof(int)) <= 0) {
        printf("[Thread %d] ", tdL.idThread);
        perror("read() error\n");
    }

    switch (REQUEST_TYPE) {
    case LOGIN: ;
        
        User u;

        if (read(sd, &u, sizeof(User)) == -1) {
            perror("[server] " READ_ERROR);
            return;
        }

        u = getUserByEmail(db, u.email); // test, need to verify if the user enter the email correctly

        if (write(sd, &u, sizeof(User)) == -1) {
            perror("[server] " WRITE_ERROR);
            return;
        }

        break;
    
    default:
        break;
    }

    printf("Request end\n\n");
}
