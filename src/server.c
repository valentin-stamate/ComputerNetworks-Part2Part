#include "import/server_static.h"

extern int errno;
#define MAX_THREAD 200

struct thData {
    int idThread;
    int sdCl;

    int user_id;
    char user_email[255];
    int isActive;
    int sdTr;
    int sdSr;
};

typedef struct thData thData;

static void *treat(void *);
void process_request(void *);

sqlite3 *db;

thData *tdDat;
int nTdData = 0;

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


        int type;
        // get the communication type 0 = normal, 1 = read file, 2 = write file
        if (read(client, &type, sizeof(int)) == -1) {
            perror(READ_ERROR);
        }

        if (type == CONNECT_TRANSFER || type == CONNECT_SEARCH) {
            User u;
            if (read(client, &u, sizeof(User)) == -1) {
                perror(READ_ERROR);
            }

            for (int i = 0; i < nTdData; i++) {
                if (tdDat[i].user_id == u.userID) {
                    if (type == CONNECT_TRANSFER) {
                        printf("Connected transfer socket\n");
                        tdDat[i].sdTr = client;
                        break;
                    }

                    if (type == CONNECT_SEARCH) {
                        printf("Connected search socket\n");
                        tdDat[i].sdSr = client;
                        break;
                    }
                }
            }

            continue;
        }

        tdDat[nTdData].idThread = nTdData;
        tdDat[nTdData].sdCl = client;

        pthread_create(&th[nTdData], NULL, &treat, tdDat + nTdData);
    
        nTdData++;
    }

    closeDatabase(db);
    return 0;
}

static void *treat(void *arg) {
    thData *tdL;
    tdL = (thData *)arg;

    printf("[thread] - %d - Waiting for the request...\n", tdL->idThread);

    pthread_detach(pthread_self());

    process_request((thData *)arg);

    close((intptr_t)arg);
    return (NULL);
};

void process_request(void *arg) {
    thData *tdL;
    tdL = (struct thData *)arg;

    int sd = tdL->sdCl;

    int REQUEST_TYPE;

    repeat:

    if (read(sd, &REQUEST_TYPE, sizeof(int)) <= 0) {
        printf("[Thread %d] ", tdL->idThread);
        perror("read() error\n");
        tdL->user_id = -1;
        return;
    }

    User u;
    User active_users[MAX_THREAD];
    int nActiveUsers;

    switch (REQUEST_TYPE) {
    case LOGIN: ;
        printf("Login Request\n");

        if (read(sd, &u, sizeof(User)) == -1) {
            perror("[server] " READ_ERROR);
            tdL->user_id = -1;
            return;
        }

        verifyUser(db, &u);

        if (u.userID != -1) {
            sprintf(tdL->user_email, "%s", u.email);
            tdL->isActive = 1;
        }

        // allow only one sesion for users
        for (int i = 0; i < nTdData && u.userID != -1; i++) {
            if (tdDat[i].user_id == u.userID) {
                u.userID = -2;
                break;
            }
        }

        tdL->user_id = u.userID != -2 ? u.userID : -1;

        if (write(sd, &u, sizeof(User)) == -1) {
            perror("[server] " WRITE_ERROR);
            tdL->user_id = -1;
            return;
        }

        break;
    case SIGNUP: ;
        printf("Signup Request\n");

        if (read(sd, &u, sizeof(User)) == -1) {
            perror("[server] " READ_ERROR);
            tdL->user_id = -1;
            return;
        }

        addUser(db, &u);

        if (u.userID != -1) {
            sprintf(tdL->user_email, "%s", u.email);
            tdL->isActive = 1;
        }

        if (write(sd, &u, sizeof(User)) == -1) {
            perror("[server] " WRITE_ERROR);
            tdL->user_id = -1;
            return;
        }

        break;
    case LOGOUT: ;
        printf("User %d disconected\n", tdL->user_id);
        tdL->isActive = 0;
        break;

    case GET_USERS: ;

        nActiveUsers = 0;

        for (int i = 0; i < nTdData; i++) {
            if (tdDat[i].user_id != -1 && tdDat[i].user_id != tdL->user_id) {
                User t = getUserByEmail(db, tdDat[i].user_email);
                sprintf(t.password, "%s", "");
                t.isActive = tdDat[i].isActive;
                active_users[nActiveUsers++] = t;
            }
        }

        if (write(sd, &nActiveUsers, sizeof(int)) == -1) {
            printf("[SHOW_ACTIVE_USERS]" WRITE_ERROR);
            tdL->user_id = -1;
            return;
        }

        for (int i = 0; i < nActiveUsers; i++) {
            if (write(sd, active_users + i, sizeof(User)) == -1) {
                printf("[SHOW_ACTIVE_USERS]" WRITE_ERROR);
                tdL->user_id = -1;
                return;
            }
        }

        break;

    case GET_FILE: ;

        RequestedFile rf;

        if (read(sd, &rf, sizeof(RequestedFile)) == -1) {
            perror("[TRANSFER]" READ_ERROR);
            tdL->user_id = -1;
            return;
        }

        int sdTr;

        for (int i = 0; i < nTdData; i++) {
            if (tdDat[i].user_id == rf.user_id) {
                sdTr = tdDat[i].sdTr;
                break;
            }
        }
        
        if (write(sdTr, &rf, sizeof(RequestedFile)) == -1) {
            perror("[TRANSFER]" WRITE_ERROR);
            tdL->user_id = -1;
            return;
        }
        
        char buffer[4096];
        int bytes;

        while (1) {
            // get bytes
            if (read(sdTr, &bytes, sizeof(int)) == -1) {
                perror("[TRANSFER]" READ_ERROR);
                tdL->user_id = -1;
                return;
            }
            // sent to user bytes lenght
            if (write(sd, &bytes, sizeof(int)) == -1) {
                perror("[TRANSFER]" WRITE_ERROR);
                tdL->user_id = -1;
                return;
            }

            if (bytes == 0) {
                break;
            }

            // read the buffer
            if (read(sdTr, buffer, bytes) == -1) {
                perror("[TRANSFER]" READ_ERROR);
                tdL->user_id = -1;
                return;
            }

            // send to user the buffer
            if (write(sd, buffer, bytes) == -1) {
                perror("[TRANSFER]" WRITE_ERROR);
                tdL->user_id = -1;
                return;
            }

        }



        break;

    case SEARCH_USER_FILES: ;
 
        SearchFile sf;

        if (read(sd, &sf, sizeof(SearchFile)) == -1) {
            perror("[SEARCH USER FILES]" READ_ERROR);
            tdL->user_id = -1;
            return;
        }

        int sdSr;

        for (int i = 0; i < nTdData; i++) {
            if (tdDat[i].user_id == sf.user_id) {
                sdSr = tdDat[i].sdSr;
                break;
            }
        }

        if (write(sdSr, &sf, sizeof(SearchFile)) == -1) {
            perror("[SEARCH USER FILES]" WRITE_ERROR);
            tdL->user_id = -1;
            return;
        }

        int nFiles;

        if (read(sdSr, &nFiles, sizeof(int)) == -1) {
            perror("[SEARCH USER FILES]" READ_ERROR);
            tdL->user_id = -1;
            return;
        }

        if (write(sd, &nFiles, sizeof(int)) == -1) {
            perror("[SEARCH USER FILES]" WRITE_ERROR);
            tdL->user_id = -1;
            return;
        }

        for (int i = 0; i < nFiles; i++) {
            File f;
            if (read(sdSr, &f, sizeof(File)) == -1) {
                perror("[SEARCH USER FILES]" READ_ERROR);
                tdL->user_id = -1;
                return;
            }
            if (write(sd, &f, sizeof(File)) == -1) {
                perror("[SEARCH USER FILES]" WRITE_ERROR);
                tdL->user_id = -1;
                return;
            }
        }

        break;

    default:
        break;
    }

    printf("Request end\n\n");

    goto repeat;
}
