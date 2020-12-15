#include "import/client_static.h"
extern int errno;

int isLogged = 0;

User *user;
File user_files[100];

char notifications[MAX_NOTIF][100];
int nNotif = 0;

static void *treat(void*);
void process_file_transfer(int*);

int connectedUsers[15];
int nCn = 0;

int currentThread = 1;

int sd, sdFileTransfer;
struct sockaddr_in server;

int uploading = 0;

void initializeTransferDescriptors(int, int*);

int main(int argc, char *argv[]) {


    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(GATEWAY_IP);
    server.sin_port = htons(PORT);
    
    // 

    user = (User*) malloc(sizeof(User));
    user->userID = -1;

    char rawCommand[100];
    char SIGGNED_AS[300] = "";

    system("clear");

    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        perror (SOCKET_ERROR);
        return errno;
    }

    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1) {
        perror (CONNECT_ERROR);
        return errno;
    }

    int connType = 0;
    if (write(sd, &connType, sizeof(int)) == -1) {
        perror(WRITE_ERROR);
    }

    pthread_t thF;
    pthread_create(&thF, NULL, &treat, &sdFileTransfer);

    repeat:
    
    system("clear");
    showWelcomeMessage(user);
    showNotifications(notifications, nNotif);

    sprintf(SIGGNED_AS, "%s", "");
    if (isLogged == 1) {
        sprintf(SIGGNED_AS, BBLU "[" BGRN "%s" BBLU "]" reset, user->username);
    }

    printf("%s" CLI " ", SIGGNED_AS);
    getLine(rawCommand, sizeof(rawCommand));


    trimString(rawCommand, ' ');

    char command[10][100];
    int blocks = 0;

    getBlocks(command, rawCommand, &blocks);

    int COMMAND_TYPE = process(command, blocks);
    
    char buffer[4096];
    
    switch (COMMAND_TYPE) {
    case LOGIN:
        clearNotifications(notifications, &nNotif);
        
        sendLoginCredentials(sd, command, user);

        isLogged = (user->userID != -1);
    
        if (isLogged == 1) {
            // printf("Logged in. Welcome " BGRN "%s.\n\n" reset, user->username);
            initializeTransferDescriptors(sd, &sdFileTransfer);
        } else {
            // printf("Invalid credentials\n\n");
        }

        break;
    case SIGNUP:
        clearNotifications(notifications, &nNotif);

        getUserCredentials(sd, user);

        isLogged = (user->userID != -1);

        if (isLogged == 1) {
            initializeTransferDescriptors(sd, &sdFileTransfer);
            // printf("Successfully signed in. Welcome " BCYN "%s.\n\n" reset, user->username);
        } else {
            // printf("Invalid credentials. User may already exist.\n\n");
        }

        break;
    case LOGOUT:
        clearNotifications(notifications, &nNotif);

        // printf("Logged out\n\n");

        isLogged = 0;
        user->userID = -1;
        nNotif = 0;

        int r = LOGOUT;
        if (write(sd, &r, sizeof(int)) == -1) {
            printf("[LOGOUT] " WRITE_ERROR "\n");
        }

        break;

    case GET_USERS: ;
        clearNotifications(notifications, &nNotif);

        if (isLogged == 0) {
            // TODO display message
            break;
        }

        getUsers(sd, notifications, &nNotif);

        break;

    case SHOW_FILES: ;
        clearNotifications(notifications, &nNotif);

        if (isLogged == 0) {
            // TODO display message
            break;
        }

        char files[100][100];
        int n = 0;

        MyFind(FILES_LOCATION, files, &n);

        for (int i = 0; i < n; i++) {
            pushNotification(files[i], notifications, &nNotif);
        }

        break;
    
    case GET_FILE: ;

        printf("Getting file. Waiting for the client to confirm the tranfer.\n");

        RequestedFile rf;
        rf.user_id = 1;
        sprintf(rf.filePath, "./files/file1.txt");

        int type = GET_FILE;
        write(sd, &type, sizeof(int));

        
        write(sd, &rf, sizeof(RequestedFile));
        
        // suppose the files have maximum 4kb
        read(sd, buffer, 4096);

        printf("Buffer is %s\n", buffer);

        scanf("%d", &type);

        break;

    case PUT_FILE: ;

        printf("Waiting for the client to request a file...\n");
        uploading = 1;
        sleep(3);

        while (uploading == 1) {
            sleep(1);
        }

        printf("Done\n");
        int fds;
        scanf("%d", &fds);

        break;

    default:
        printf("Invalid command! To see all commands press " BWHT "help.\n\n" reset);
        break;
    }


    goto repeat;

    close (sd);

    return 0;
}

static void *treat(void *arg) {
    int *sd = (int*)arg;

    pthread_detach(pthread_self());

    process_file_transfer(sd);

    close((intptr_t)arg);
    return (NULL);
};

RequestedFile rf;
void process_file_transfer(int *arg) {
    
    repeat:

    while (uploading == 0) {
        sleep(1);
    }

    read(sdFileTransfer, &rf, sizeof(RequestedFile));

    int fd = open(rf.filePath, O_RDONLY);
    char buffer[4096];
    read(fd, buffer, 4096);

    write(sdFileTransfer, buffer, 4096);

    uploading = 0;

    goto repeat;
}

struct sockaddr_in socket_file;

void initializeTransferDescriptors(int sd, int* sdF) {

    int type = 1;

    socket_file.sin_family = AF_INET;
    socket_file.sin_addr.s_addr = inet_addr(GATEWAY_IP);
    socket_file.sin_port = htons(PORT);

    if (((*sdF) = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        perror (SOCKET_ERROR);
        return;
    }

    if (connect ((*sdF), (struct sockaddr *) &socket_file,sizeof (struct sockaddr)) == -1) {
        perror (CONNECT_ERROR);
        return;
    }

    write((*sdF), &type, sizeof(int));
    write((*sdF), user, sizeof(User));

}





