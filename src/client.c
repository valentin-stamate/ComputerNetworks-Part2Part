#include "import/client_static.h"
extern int errno;

int isLogged = 0;

User *user;
File user_files[100];

char notifications[MAX_NOTIF][500];
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
RequestedFile rf;

int main(int argc, char *argv[]) {


    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(GATEWAY_IP);
    server.sin_port = htons(PORT);

    user = (User*) malloc(sizeof(User));
    user->userID = -1;

    char rawCommand[100];
    char SIGGNED_AS[300] = "";

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
    clearNotifications(notifications, &nNotif);

    sprintf(SIGGNED_AS, "%s", "");
    if (isLogged == 1) {
        sprintf(SIGGNED_AS, BBLU "[" BGRN "%s" BBLU "]" reset, user->username);
    }

    printf("%s" CLI " ", SIGGNED_AS);
    getLine(rawCommand, sizeof(rawCommand));


    trimString(rawCommand, ' ');

    char command[10][255];
    int blocks = 0;

    getBlocks(command, rawCommand, &blocks);

    int COMMAND_TYPE = process(command, blocks);
    
    char buffer[4096];
    char tempLine[500];

    switch (COMMAND_TYPE) {
    case LOGIN:
        
        sendLoginCredentials(sd, command, user);

        isLogged = (user->userID != -1);
    
        if (isLogged == 1) {
            sprintf(tempLine, "Logged in. Welcome " BGRN "%s." reset, user->username);
            pushNotification(tempLine, notifications, &nNotif);
            
            initializeTransferDescriptors(sd, &sdFileTransfer);
        } else {
            pushNotification(BRED "Invalid credentials" reset, notifications, &nNotif);
        }

        break;
    case SIGNUP:

        getUserCredentials(sd, user);

        isLogged = (user->userID != -1);

        if (isLogged == 1) {
            initializeTransferDescriptors(sd, &sdFileTransfer);
            
            sprintf(tempLine, BWHT "Successfully signed in. Welcome " BCYN "%s." reset, user->username);
            pushNotification(tempLine, notifications, &nNotif);
        } else {
            sprintf(tempLine, BWHT "Invalid credentials. User may already exist." reset);
            pushNotification(tempLine, notifications, &nNotif);
        }

        break;
    case CLEAR_NOTIFICATIONS: ;
        clearNotifications(notifications, &nNotif);
        break;
    case LOGOUT:
        pushNotification(BWHT "Logged out." reset, notifications, &nNotif);

        isLogged = 0;
        user->userID = -1;
        nNotif = 0;

        int r = LOGOUT;
        if (write(sd, &r, sizeof(int)) == -1) {
            printf("[LOGOUT] " WRITE_ERROR "\n");
        }

        break;

    case GET_USERS: ;

        if (isLogged == 0) {
            pushNotification(BWHT "In order to run this command log in first." reset, notifications, &nNotif);
            break;
        }

        getUsers(sd, notifications, &nNotif);

        break;

    case SHOW_FILES: ;

        if (isLogged == 0) {
            pushNotification(BWHT "In order to run this command log in first." reset, notifications, &nNotif);
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
        // for testing purposes
        rf.user_id = 1;
        sprintf(rf.username, "ValentinSt");
        sprintf(rf.filePath, "./files/file1.txt");

        int type = GET_FILE;
        if (write(sd, &type, sizeof(int)) == -1) {
            perror("[GETTING FILE]" WRITE_ERROR);
        }

        if (write(sd, &rf, sizeof(RequestedFile)) == -1) {
            perror("[GETTING FILE]" WRITE_ERROR);
        }
        
        // suppose the files have maximum 4kb
        if (read(sd, buffer, 4096) == -1) {
            perror("[GETTING FILE]" READ_ERROR);
        }

        printf("Buffer is %s\n", buffer);

        sprintf(tempLine, BGRN "File transfered succesfully from " BWHT "%s" BGRN "." reset, rf.username);
        pushNotification(tempLine, notifications, &nNotif);

        sleep(5);// for testing purposes

        break;

    case PUT_FILE: ;

        printf("Waiting for the client to request a file...\n");
        uploading = 1;
        sleep(3);

        while (uploading == 1) {
            sleep(1);
        }

        sprintf(tempLine, BGRN "File transfered succesfully to " BWHT "%s" BGRN "." reset, rf.username);
        pushNotification(tempLine, notifications, &nNotif);

        break;

    default:
        pushNotification("Invalid command! To see all commands press " BWHT "help." reset, notifications, &nNotif);
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

void process_file_transfer(int *arg) {
    
    repeat:

    while (uploading == 0) {
        sleep(1);
    }

    if (read(sdFileTransfer, &rf, sizeof(RequestedFile)) == -1) {
        perror("[PR FILE]" READ_ERROR);
    }

    int fd = open(rf.filePath, O_RDONLY);
    
    char buffer[4096];
    if (read(fd, buffer, 4096) == -1) {
        perror("[READING FILE]" READ_ERROR);
    }

    if (write(sdFileTransfer, buffer, 4096) == -1) {
        perror("[WRITING BUFFER]" WRITE_ERROR);
    }

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





