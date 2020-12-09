#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <netdb.h>
#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <pthread.h>

#include "import/sql.h"

#define MAX_FILES 100

#define PORT 2024
#define GATEWAY_IP "192.168.1.9"
#define LOCAL_IP "127.0.0.1"

extern int errno;

typedef struct thData {
    int idThread;
    int cl;    
} thData;

static void *treat(void *);
void raspunde(void *);

int main(int argc, char *argv[]) {

    sqlite3 *db = openDatabase(DATABASE);

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
        perror("[server]Eroare la socket().\n");
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
        perror("[server]Eroare la bind().\n");
        return errno;
    }

    if (listen(sd, 2) == -1) {
        perror("[server]Eroare la listen().\n");
        return errno;
    }

    while (1) {
        int client;
        thData *td;
        int length = sizeof(from);

        printf("[server]Asteptam la portul %d...\n", PORT);
        fflush(stdout);

        if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0) {
            perror("[server]Eroare la accept().\n");
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
    printf("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
    fflush(stdout);
    pthread_detach(pthread_self());
    raspunde((struct thData *)arg);

    close((intptr_t)arg);
    return (NULL);
};

void raspunde(void *arg) {
    int nr, i = 0;
    struct thData tdL;
    tdL = *((struct thData *)arg);
    if (read(tdL.cl, &nr, sizeof(int)) <= 0) {
        printf("[Thread %d]\n", tdL.idThread);
        perror("Eroare la read() de la client.\n");
    }

    printf("[Thread %d]Mesajul a fost receptionat...%d\n", tdL.idThread, nr);

    nr++;
    printf("[Thread %d]Trimitem mesajul inapoi...%d\n", tdL.idThread, nr);

    if (write(tdL.cl, &nr, sizeof(int)) <= 0) {
        printf("[Thread %d] ", tdL.idThread);
        perror("[Thread]Eroare la write() catre client.\n");
    }
    else
        printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
}
