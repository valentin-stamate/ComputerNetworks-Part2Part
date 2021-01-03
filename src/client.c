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

void pushNotif(char*);
void showNotif();
void clearNotif();

int helpShown = 0;

int sd, sdFt, sdSr;
struct sockaddr_in server;

int uploading = 0;
int searching = 0;

RequestedFile rf;
SearchFile sf;

int main(int argc, char *argv[]) {

    char* ip;
    int port;

    if (argc == 3) {
        ip = argv[1];
        port = atoi(argv[2]);
    } else {
        ip = DEFAULT_GATEWAY_IP;
        port = atoi(DEFAULT_PORT);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = htons(port);
    
    printf("Connected with ip address: %s on port %d\n", ip, port);

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
    showNotif();
    clearNotif();

    sprintf(SIGGNED_AS, "%s", "");
    if (isLogged == 1) {
        sprintf(SIGGNED_AS, BBLU "[" BGRN "%s" BBLU "]" reset, user->username);
    }

    printf("%s" CLI " ", SIGGNED_AS);
    getLine(rawCommand, sizeof(rawCommand));

    if (helpShown == 1) {
        helpShown = 0;
        goto repeat;
    }

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
            pushNotif(BYEL "Only one session is allowed per user" reset);
            break;
        }

        if (isLogged == 1) {
            sprintf(tempLine, "Logged in. Welcome " BGRN "%s." reset, user->username);
            pushNotif(tempLine);
            
            initializeTransferDescriptors(sd, &sdFt, &sdSr, user, ip, port);
        } else {
            pushNotif(BRED "Invalid credentials" reset);
        }

        break;
    case SIGNUP:

        getUserCredentials(sd, user);

        isLogged = (user->userID != -1);

        if (isLogged == 1) {
            initializeTransferDescriptors(sd, &sdFt, &sdSr, user, ip, port);
            
            sprintf(tempLine, BWHT "Successfully signed in. Welcome " BCYN "%s." reset, user->username);
            pushNotif(tempLine);
        } else {
            sprintf(tempLine, BWHT "Invalid credentials. User may already exist." reset);
            pushNotif(tempLine);
        }

        break;
    case HELP:

        showHelp();
        helpShown = 1;

        break;
    case CLEAR_NOTIFICATIONS: ;
        clearNotifications(notifications, &nNotif);
        break;
    case LOGOUT:

        if (isLogged == 0) {
            pushNotif(BWHT "In order to run this command log in first." reset);
            break;
        }

        pushNotif(BWHT "Logged out." reset);

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
            pushNotif(BWHT "In order to run this command log in first." reset);
            break;
        }

        if (ncUsers == 0) {
            pushNotif(BWHT "You don't have user connections");
        }

        for (int i = 0; i < ncUsers; i++) {
            sprintf(tempLine, BWHT "%s" reset, cUsers[i].username);
            pushNotif(tempLine);
        }

        break;
    case GET_USERS: ;

        if (isLogged == 0) {
            pushNotif(BWHT "In order to run this command log in first." reset);
            break;
        }

        getUsers(sd, notifications, &nNotif, aUsers, &naUsers);

        break;
    case CONNECT_TO: ;

        if (isLogged == 0) {
            pushNotif(BWHT "In order to run this command log in first." reset);
            break;
        }

        if (naUsers == 0) {
            pushNotif(BWHT "Run " BMAG "show users" BWHT " first." reset);
            break;
        }

        int connectToID = atoi(command[2]);

        for (int i = 0; i < ncUsers; i++) {
            if (cUsers[i].userID == connectToID) {
                sprintf(tempLine, BYEL "Already connected to user" BWHT "%s" BYEL "." reset, cUsers->username);
                pushNotif(tempLine);
                break;
            }
        }

        for (int i = 0; i < naUsers; i++) {
            if (aUsers[i].userID == connectToID) {
                cUsers[ncUsers++] = aUsers[i];
                sprintf(tempLine, BWHT "Successfully connected to " BGRN "%s" BWHT "." reset, cUsers[ncUsers - 1].username);
                pushNotif(tempLine);
                sprintf(tempLine, "%s", BWHT "Now you can search files by typing search [user_id][(sub)name][.ext][-/+size]|[]." reset);
                pushNotif(tempLine);
                break;
            }
        }
        
        break;
    case SHOW_FILES: ;

        if (isLogged == 0) {
            pushNotif(BWHT "In order to run this command log in first." reset);
            break;
        }

        n_uf = 0;

        MyFind(FILES_LOCATION, user_files, &n_uf, NULL);

        if (n_uf == 0) {
            pushNotif(BWHT "No files were found locally." reset);
        }

        for (int i = 0; i < n_uf; i++) {
            pushNotif(user_files[i].path);
        }

        break;
    case SHOW_DOWNLOADED_FILES: ;
        if (isLogged == 0) {
            pushNotif(BWHT "In order to run this command log in first." reset);
            break;
        }

        n_uf = 0;

        MyFind(DOWNLOAD_PATH, user_files, &n_uf, NULL);

        if (n_uf == 0) {
            pushNotif(BWHT "No downloaded files were found." reset);
        }

        for (int i = 0; i < n_uf; i++) {
            pushNotif(user_files[i].path);
        }

        break;
    case SEARCH_USER_FILES: ;

        if (isLogged == 0) {
            pushNotif(BWHT "In order to run this command log in first." reset);
            break;
        }

        int u_id = atoi(command[1]);

        int found = 0;
        for (int i = 0; i < ncUsers; i++) {
            found = found || u_id == cUsers[i].userID;
        }

        if (found == 0) {
            pushNotif(BWHT "No users found to search files" reset);
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
            pushNotif(BWHT "No files were found." reset);
            break;
        }

        for (int i = 0; i < n_ouf; i++) {

            if (read(sd, other_user_files + i, sizeof(File)) == -1) {
                perror("[Search User File]" READ_ERROR);
                return 0;
            }

            sprintf(tempLine, BWHT "Filename: " BBLU "%s" BWHT " with id %d" reset, other_user_files[i].name, i);
            pushNotif(tempLine);
        }

        pushNotif( BWHT "To get a file run " BMAG "get file [file_id]" BWHT "." reset);

        break;

    case ALLOW_DISCOVERY: ;

        if (isLogged == 0) {
            pushNotif(BWHT "In order to run this command log in first." reset);
            break;
        }

        printf("Waiting for the client to search a file...\n");
        
        searching = 1;

        while (searching == 1) {
            sleep(1);
        }
        // TODO put a notification
        break;

    case GET_FILE: ;

        if (isLogged == 0) {
            pushNotif(BWHT "In order to run this command log in first." reset);
            break;
        }

        printf("Getting file. Waiting for the client to confirm the tranfer...\n");

        int selected_file = atoi(command[2]);

        if (n_ouf == 0) {
            pushNotif(BWHT "Select search for files first." reset);
            break;
        }

        if (selected_file >= n_ouf) {
            pushNotif(BWHT "Select a valid file_id" reset);
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
        mkdir(DOWNLOAD_PATH, 0777);

        char path[4096];
        sprintf(path, "%s/%s", DOWNLOAD_PATH, rf.fileName);

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

        sprintf(tempLine, BGRN "File " BMAG "%s" BGRN " transfered succesfully." reset, rf.fileName);
        pushNotif(tempLine);

        sleep(5);// for testing purposes

        break;

    case SEND_FILE: ;

        if (isLogged == 0) {
            pushNotif(BWHT "In order to run this command log in first." reset);
            break;
        }

        printf("Waiting for the client to request a file...\n");
        uploading = 1;
        sleep(3);

        while (uploading == 1) {
            sleep(1);
        }

        sprintf(tempLine, BGRN "File transfered succesfully." reset);
        pushNotif(tempLine);

        break;

    default:
        pushNotif("Invalid command! To see all commands press " BWHT "help." reset);
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

void pushNotif(char* text) {
    pushNotification(text, notifications, &nNotif);
}

void showNotif() {
    showNotifications(notifications, nNotif);
}

void clearNotif() {
    clearNotifications(notifications, &nNotif);
}




