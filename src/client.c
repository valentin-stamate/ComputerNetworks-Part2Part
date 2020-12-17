#include "import/client_static.h"
extern int errno;

int isLogged = 0;

User *user;

File user_files[MAX_FILES];
int n_uf = 0;

File other_user_files[MAX_FILES];
int n_ouf = 0;

User aUsers[30];
int naUsers = 0;

User cUsers[30];
int ncUsers = 0;

char notifications[MAX_NOTIF][500];
int nNotif = 0;

static void *treat(void*);
void process_file_transfer(int*);

static void *treat_search(void*);
void process_search(int*);

int nCn = 0;

int currentThread = 1;

int sd, sdFt, sdSr;
struct sockaddr_in server;

int uploading = 0;
int searching = 0;

void initializeTransferDescriptors(int, int*, int*);
RequestedFile rf;
SearchFile sf;

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
    pthread_create(&thF, NULL, &treat, &sdFt);

    pthread_t thSr;
    pthread_create(&thF, NULL, &treat_search, &sdSr);

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
    
    unsigned char buffer[4096];
    char tempLine[500];
    int type;
    switch (COMMAND_TYPE) {
    case LOGIN:
        
        sendLoginCredentials(sd, command, user);

        isLogged = (user->userID != -1);
    
        if (user->userID == -2) {
            isLogged = 0;
            pushNotification(BYEL "Only one session is allowed per user" reset, notifications, &nNotif);
            break;
        }

        if (isLogged == 1) {
            sprintf(tempLine, "Logged in. Welcome " BGRN "%s." reset, user->username);
            pushNotification(tempLine, notifications, &nNotif);
            
            initializeTransferDescriptors(sd, &sdFt, &sdSr);
        } else {
            pushNotification(BRED "Invalid credentials" reset, notifications, &nNotif);
        }

        break;
    case SIGNUP:

        getUserCredentials(sd, user);

        isLogged = (user->userID != -1);

        if (isLogged == 1) {
            initializeTransferDescriptors(sd, &sdFt, &sdSr);
            
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
    
    case SHOW_CONNECTED_USERS: ;

        if (isLogged == 0) {
            pushNotification(BWHT "In order to run this command log in first." reset, notifications, &nNotif);
            break;
        }

        if (ncUsers == 0) {
            pushNotification(BWHT "You don't have user connections", notifications, &nNotif);
        }

        for (int i = 0; i < ncUsers; i++) {
            sprintf(tempLine, BWHT "%s" reset, cUsers[i].username);
            pushNotification(tempLine, notifications, &nNotif);
        }

        break;
    case GET_USERS: ;

        if (isLogged == 0) {
            pushNotification(BWHT "In order to run this command log in first." reset, notifications, &nNotif);
            break;
        }

        getUsers(sd, notifications, &nNotif, aUsers, &naUsers);

        break;
    case CONNECT_TO: ;
        if (isLogged == 0) {
            pushNotification(BWHT "In order to run this command log in first." reset, notifications, &nNotif);
            break;
        }

        if (naUsers == 0) {
            pushNotification(BWHT "Run the command get users first" reset, notifications, &nNotif);
            break;
        }

        int connectToID = atoi(command[2]);
        for (int i = 0; i < naUsers; i++) {
            if (aUsers[i].userID == connectToID) {
                cUsers[ncUsers++] = aUsers[i];
                sprintf(tempLine, BWHT "Successfully connected to " BGRN "%s" BWHT "." reset, cUsers[ncUsers - 1].username);
                pushNotification(tempLine, notifications, &nNotif);
                pushNotification( BWHT "In order to transfer files you both have to be connected." reset, notifications, &nNotif);
                break;
            }
        }
        
        break;
    case SHOW_FILES: ;

        if (isLogged == 0) {
            pushNotification(BWHT "In order to run this command log in first." reset, notifications, &nNotif);
            break;
        }

        n_uf = 0;
        MyFind(FILES_LOCATION, user_files, &n_uf, NULL);

        for (int i = 0; i < n_uf; i++) {
            pushNotification(user_files[i].path, notifications, &nNotif);
        }

        break;
    
    case SEARCH_USER_FILES: ;

        int u_id = atoi(command[1]);

        int found = 0;
        for (int i = 0; i < ncUsers; i++) {
            found = found || u_id == cUsers[i].userID;
        }

        if (found == 0) {
            pushNotification(BWHT "No users found to search files" reset, notifications, &nNotif);
            break;
        }

        printf("Waiting for the user to accept\n");        

        sprintf(sf.params, "%s", command[2]);
        sf.user_id = u_id;
        
        type = SEARCH_USER_FILES;
        if (write(sd, &type, sizeof(int)) == -1 ) {
            perror("[Search User File]" WRITE_ERROR);
            return 0;
        }

        if (write(sd, &sf, sizeof(SearchFile)) == -1) {
            perror("[Search User File]" WRITE_ERROR);
            return 0;
        }

        if (read(sd, &n_ouf, sizeof(int)) == -1) {
            perror("[Search User File]" READ_ERROR);
            return 0;
        }

        if (n_ouf == 0) {
            pushNotification(BWHT "" reset, notifications, &nNotif);
            break;
        }

        for (int i = 0; i < n_ouf; i++) {

            if (read(sd, other_user_files + i, sizeof(File)) == -1) {
                perror("[Search User File]" READ_ERROR);
                return 0;
            }

            sprintf(tempLine, BWHT "Filename: " BMAG "%s" BWHT " with id %d" reset, other_user_files[i].name, i);
            pushNotification(tempLine, notifications, &nNotif);
        }

        pushNotification( BLKB "The command to get a file is " BBLU "get [file_id]" reset, notifications, &nNotif);

        break;

    case ALLOW_DISCOVERY: ;
        printf("Waiting for the client to search a file...\n");
        
        searching = 1;

        while (searching == 1) {
            sleep(1);
        }
        // TODO put a notification
        break;

    case GET_FILE: ;

        printf("Getting file. Waiting for the client to confirm the tranfer.\n");

        int selected_file = atoi(command[2]);

        if (n_ouf == 0) {
            pushNotification(BWHT "Select search for files first." reset, notifications, &nNotif);
            break;
        }

        if (selected_file >= n_ouf) {
            pushNotification(BWHT "Select a valid file_id" reset, notifications, &nNotif);
            break;
        }

        rf.user_id = sf.user_id; // the user id from the last search
        sprintf(rf.username, "%s", "");
        sprintf(rf.fileName, "%s", other_user_files[selected_file].name);
        sprintf(rf.filePath, "%s", other_user_files[selected_file].path);
        
        type = GET_FILE;
        if (write(sd, &type, sizeof(int)) == -1) {
            perror("[GETTING FILE]" WRITE_ERROR);
            return 0;
        }

        if (write(sd, &rf, sizeof(RequestedFile)) == -1) {
            perror("[GETTING FILE]" WRITE_ERROR);
            return 0;
        }
        
        int bytes;
        mkdir("./downloads", 0777);

        char path[4096];
        sprintf(path, "%s/%s", "./downloads", rf.fileName);
        printf("The path is %s\n", path);

        int fdFile = open(path, O_WRONLY | O_CREAT, 0666);

        if (fdFile == -1) {
            perror("Error creating the file to download\n");
            return 0;
        }

        while (1) {
            if (read(sd, &bytes, sizeof(int)) == -1) {
                perror("[GETTING FILE]" READ_ERROR);
                return 0;
            }
            
            if (bytes == 0) {
                break;
            }
            // read chunks of 4kb
            if (read(sd, buffer, bytes) == -1) {
                perror("[GETTING FILE]" READ_ERROR);
                return 0;
            }

            if (write(fdFile, buffer, bytes) == -1) {
                perror("Error writing to file\n");
                return 0;
            }
        }


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

        sprintf(tempLine, BGRN "File transfered succesfully." reset);
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

    if (read(sdFt, &rf, sizeof(RequestedFile)) == -1) {
        perror("[PR FILE]" READ_ERROR);
    }

    int fd = open(rf.filePath, O_RDONLY);
    
    if (fd == -1) {
        perror("[FILE ERROR]" OPEN_ERROR);
        return;
    }

    unsigned char buffer[4096];

    while (1) {
        int bytes = read(fd, buffer, 4096);

        if (bytes == -1) {
            perror("[READING FILE]" READ_ERROR);
            return;
        }

        if (write(sdFt, &bytes, sizeof(int)) == -1) {
            perror("[WRITING BUFFER]" WRITE_ERROR);
            return;
        }

        if (bytes == 0) {    
            break;
        }

        if (write(sdFt, buffer, bytes) == -1) {
            perror("[WRITING BUFFER]" WRITE_ERROR);
            return;
        }
    }


    uploading = 0;

    goto repeat;
}

static void *treat_search(void *arg) {
    int *sd = (int*)arg;

    pthread_detach(pthread_self());

    process_search(sd);

    close((intptr_t)arg);
    return (NULL);
};

void process_search(int *arg) {
    
    repeat:

    while (searching == 0) {
        sleep(1);
    }

    SearchFile sf;

    if (read(sdSr, &sf, sizeof(SearchFile)) == -1) {
        perror("[PROCESS SEARCH]" READ_ERROR);
        return;
    }

    int filesFound = 0;
    File files[MAX_FILES];
    
    SearchParams sp;
    processParams(&sp, sf.params); //

    MyFind(FILES_LOCATION, files, &filesFound, &sp);

    if (write(sdSr, &filesFound, sizeof(int)) == -1) {
        perror("[PROCESS SEARCH]" WRITE_ERROR);
        return;
    }

    for (int i = 0; i < filesFound; i++) {
        if (write(sdSr, files + i, sizeof(File)) == -1) {
            perror("[PROCESS SEARCH]" WRITE_ERROR);
            return;
        }
    }

    searching = 0;
    goto repeat;
}


struct sockaddr_in socket_file;
struct sockaddr_in socket_search;

void initializeTransferDescriptors(int sd, int* sdF, int *sdSr) {

    int type = CONNECT_TRANSFER;

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


    type = CONNECT_SEARCH;

    socket_search.sin_family = AF_INET;
    socket_search.sin_addr.s_addr = inet_addr(GATEWAY_IP);
    socket_search.sin_port = htons(PORT);

    if (((*sdSr) = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        perror (SOCKET_ERROR);
        return;
    }

    if (connect ((*sdSr), (struct sockaddr *) &socket_search,sizeof (struct sockaddr)) == -1) {
        perror (CONNECT_ERROR);
        return;
    }

    write((*sdSr), &type, sizeof(int));
    write((*sdSr), user, sizeof(User));

}





