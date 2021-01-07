#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "client_functions.h"
#include "../ANSI-color-codes.h"
#include "../user_commands.h"
#include "../errors.h"
#include "../configuration.h"

void showWelcomeMessage(User* u) {
    char user[300] = "";
    if (u->userID != -1) {
        sprintf(user, "%s ", u->username);
    }
    printf(BBLU "Welcome " BGRN "%s" BBLU "to Part2Part.\n", user);

    if (u->userID == -1) {
        printf("In order to run commands log in fist.\n\n" reset);
    } else {
        printf("To see all the commands type " BMAG "help" BBLU ".\n\n" reset);
    }

    printf(BWHT "Few useful commands: " YELB " help " reset " " GRNB BWHT " login " reset " " BLUB BWHT " signup " reset " " REDB BWHT " exit " reset "\n\n");

}

void trimString(char* s, char c) {

    int i = 0;
    while (i < strlen(s)) {
        if (s[i] == c && s[i + 1] == c) {
            strcpy(s + i, s + i + 1);
            i--;
        }

        i++;
    }

    if (s[strlen(s) - 1] == c) {
        s[strlen(s) - 1] = '\0';
    }

}

void processParams(SearchParams* sp, char* params) {
    // [name][extension][(+/-)dimension]

    sp->size = 0;
    strcpy(sp->name, "");
    strcpy(sp->extension, "");

    if (strcmp(params, "[]") == 0) {
        return;
    }

    int l = 0;
    char *f = strchr(params, ']');
    l = f - params;
    strncat(sp->name, params + 1, l - 1);
    f++;

    char *s = strchr(f, ']'); 
    l = s - f;
    strncat(sp->extension, f + 1, l - 1);
    s++;

    char *t = strchr(s, ']');
    l = t - s;

    char nr[100] = "";
    strncat(nr, s + 1, l - 1);

    sp->size = atoi(nr);
}

void getBlocks(char destination[10][255], char *source, int *len) {
    char *p = strchr(source, ' ');

    *len = 0;
    while (p != NULL) {
        int blockLen = p - source;

        strcpy(destination[*len], "");
        strncat(destination[(*len)++], source, blockLen);

        strcpy(source, source + (blockLen + 1));

        p = strchr(source, ' ');
    }
    
    strcpy(destination[*len], "");
    strcat(destination[(*len)++], source);

}

int process(char command[10][255], int blocks) {
    if (blocks == 1 && strcmp(command[0], "help") == 0) {
        return HELP;
    }

    if (blocks == 1 && strcmp(command[0], "login") == 0) {
        return LOGIN;
    }

    if (blocks == 1 && strcmp(command[0], "signup") == 0) {
        return SIGNUP;
    }

    if (blocks == 1 && strcmp(command[0], "logout") == 0) {
        return LOGOUT;
    }

    if (blocks == 2 && strcmp(command[0], "allow") == 0 && strcmp(command[1], "discovery") == 0) {
        return ALLOW_DISCOVERY;
    }

    if (blocks == 3 && strcmp(command[0], "connect") == 0 && strcmp(command[1], "to") == 0) {
        return CONNECT_TO;
    }

    if (blocks == 3 && strcmp(command[0], "show") == 0 && strcmp(command[1], "downloaded") == 0 && strcmp(command[2], "files") == 0) {
        return SHOW_DOWNLOADED_FILES;
    }

    if (blocks == 2 && strcmp(command[0], "clear") == 0 && strcmp(command[1], "notifications") == 0) {
        return CLEAR_NOTIFICATIONS;
    }

    if (blocks == 3 && strcmp(command[0], "show") == 0 && strcmp(command[1], "connected") == 0 && strcmp(command[2], "users") == 0) {
        return SHOW_CONNECTED_USERS;
    }

    if ( blocks == 2 && strcmp(command[0], "show") == 0 && strcmp(command[1], "users") == 0 ) {
        return GET_USERS;
    }

    if (blocks == 2 && strcmp(command[0], "show") == 0 && strcmp(command[1], "files") == 0) {
        return SHOW_FILES;
    }

    if (blocks == 3 && strcmp(command[0], "search") == 0) {
        return SEARCH_USER_FILES;
    }

    if (blocks == 3 && strcmp(command[0], "get") == 0 && strcmp(command[1], "file") == 0) {
        return GET_FILE;
    }

    if (blocks == 2 && strcmp(command[0], "send") == 0 && strcmp(command[1], "file") == 0) {
        return SEND_FILE;
    }

    if (blocks == 1 && strcmp(command[0], "exit") == 0) {
        printf("Process killed\n");
        exit(1);
    }

    return ERROR;
}

void sendLoginCredentials(int sd, char command[10][255], User* u) {
    
    int type = LOGIN;
    if (write(sd, &type, sizeof(int)) == -1) {
        printf("[LOGIN 1] " WRITE_ERROR "\n");
        return;
    }
    
    printf("Email: " BYEL);
    getLine(u->email, 255);
    printf(reset "Password: " BBLK);
    getLine(u->password, 255);
    printf(reset);

    if (write(sd, u, sizeof(User)) == -1) {
        printf("[LOGIN 2] " WRITE_ERROR "\n");
        return;
    }

    if (read(sd, u, sizeof(User)) == -1) {
        printf("[LOGIN] " READ_ERROR "\n");
        return;
    }
}

void getUserCredentials(int sd, User* u) {

    int type = SIGNUP;
    if (write(sd, &type, sizeof(int)) == -1) {
        printf("[LOGIN 1] " WRITE_ERROR "\n");
        return;
    }

    printf("Username: ");
    getLine(u->username, 255);
    printf("Email: ");
    getLine(u->email, 255);
    printf("Password: ");
    getLine(u->password, 255);

    if (write(sd, u, sizeof(User)) == -1) {
        printf("[SIGNUP]" WRITE_ERROR);
        return;
    }

    if (read(sd, u, sizeof(User)) == -1) {
        printf("[SIGNUP]" READ_ERROR);
    }

}

void getUsers(int sd, char notification[MAX_NOTIF][500], int* n, User* users, int* nUsers) {
    int type = GET_USERS;
    if (write(sd, &type, sizeof(int)) == -1) {
        printf("[LOGIN 1] " WRITE_ERROR "\n");
        return;
    }

    if (read(sd, nUsers, sizeof(int)) == -1) {
        perror("[GET USERS]" READ_ERROR);
        return;
    }

    for (int i = 0; i < *nUsers; i++) {
        if (read(sd, users + i, sizeof(User)) == -1) {
            perror("[GET USERS]" READ_ERROR);
        }

        char* line = malloc(500);

        if (users[i].isActive == 1) {
            sprintf(line, BWHT "%s" reset " with id " BWHT "%d" reset " is " BGRN "active" BWHT "." reset, users[i].username, users[i].userID);
        } else {
            sprintf(line, BWHT "%s" reset " with id " BWHT "%d" reset " is " BYEL "offline" BWHT "." reset, users[i].username, users[i].userID);
        }

        pushNotification(line, notification, n);

        free(line);
    }

    char* line = malloc(500);

    if (*nUsers != 0) {
        sprintf(line, BWHT "To connect with a user type the command " BMAG "connect to [user_id]" BWHT "." reset);

        pushNotification(line, notification, n);

    } else {
        sprintf(line, "No active users found");

        pushNotification(line, notification, n);
    }

    free(line);
}

void getLine(char* buffer, int n) {
    fgets(buffer, n, stdin);
    buffer[(int)strlen(buffer) - 1] = '\0'; // removing newline
}

void showNotifications(char notification[MAX_NOTIF][500], int n) {
    for (int i = 0; i < n; i++) {
        showNotification(notification[i]);
    }
    if (n != 0) {
        printf("===================================================================================================\n\n");
    }
}

void showNotification(char* s) {
    int cut = 100 - 6;
    int nRows = strlen(s + 1) / cut + 1;

    char rowText[10][75];

    for (int i = 0; i < nRows - 1; i++) {
        strcpy(rowText[i], "");

        strncat(rowText[i], s + i * cut, cut);

    }

    sprintf(rowText[nRows - 1], "%s", s + (nRows - 1) * cut);

    printf("===================================================================================================\n");
    for (int i = 0; i < nRows - 1; i++) {
        printf("  %s  \n", rowText[i]);
    }

    printf("  %s", rowText[nRows - 1]);
    int r = strlen(rowText[nRows - 1]);
    r = cut - r + 2;

    for (int i = 1; i <= r; i++) {
        printf(" ");
    }
    printf("\n");

}

void clearNotifications(char notification[MAX_NOTIF][500], int* n) {
    while ((*n) > 0) {
        popNotification(notification, n);
    }
}

void pushNotification(char* newNot, char notifications[MAX_NOTIF][500], int* n) {
    sprintf(notifications[(*n)++], "%s", newNot);
}

void popNotification(char notifications[MAX_NOTIF][500], int* n) {
    for (int i = 0; i < 9; i++) {
        sprintf(notifications[i], "%s", notifications[i + 1]);
    }
    sprintf(notifications[9], "%s", "");
    (*n) = (*n) - 1;
}

void showHelp() {
    printf("                                          User Commands                                          \n");
    printf(BWHT "Name\n" reset);
    printf("              " BWHT "help" reset "   - shows the information about all the commands\n\n");
    
    printf("              " BWHT "signup" reset " - user this to create an account\n\n");
    
    printf("              " BWHT "login" reset "  - if you aready have an account use this to log in and use the other\n");
    printf("                     - commands listed below\n\n");
    printf("              " BWHT "logout" reset " - logout\n\n");

    printf("              " BWHT "show files" reset " - list your local files\n\n");

    printf("              " BWHT "show downloaded files" reset " - list your local downloaded files\n\n");

    printf("              " BWHT "show connected users" reset " - list all connected users users\n\n");
    
    printf("              " BWHT "search [user_id] [name][.ext][-/+size]" reset " - this will search for user files. The name is\n");
    printf("                                                     - the (sub)name of a file, .ext is the extension\n");
    printf("                                                     - and -/+ is the size: - means less than and + more than\n");
    printf("                                                     - + more than. If you don't want to specify any\n");
    printf("                                                     - parameters put [] instead.\n\n");
    
    printf("              " BWHT "send file" reset " - allow users to get the file requested\n\n");
    
    printf("              " BWHT "show users" reset " - list connected users. Then you can connect with them.\n\n");
    
    printf("              " BWHT "connect to [user_id]" reset " - connect to a user. This will allow you to get user files.\n\n");
    
    printf("              " BWHT "get file [file_id]" reset " - select the file you want to get. file_id is the id shown after\n");
    printf("                                 search command\n\n");

    
    printf("              " BWHT "allow discovery" reset " - allow users to discover your local files\n\n");
    
    printf("              " BWHT "clear notifications" reset " - clears the notifications\n\n");
    
    printf("              " BWHT "exit" reset " - kill the process\n\n");

    char c;
    scanf("%c", &c);
}

void initializeTransferDescriptors(int sd, int* sdF, int *sdSr, User* user, char* ip, int port) {
    struct sockaddr_in socket_file;
    struct sockaddr_in socket_search;

    int type = CONNECT_TRANSFER;

    socket_file.sin_family = AF_INET;
    socket_file.sin_addr.s_addr = inet_addr(ip);
    socket_file.sin_port = htons(port);

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
    socket_search.sin_addr.s_addr = inet_addr(ip);
    socket_search.sin_port = htons(port);

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


int fileSize(char* path) {
    struct stat st;

    if (stat(path, &st) == -1) {
        perror(STAT_ERROR);
    }

    return st.st_size;
}

void MyFind(char *dirname, File* files, int *n, SearchParams* sp) {
    DIR *dd = opendir(dirname);

    struct dirent *de;
    
    if (dd != NULL) {
        de = readdir(dd);
    } else {
        return;
    }

    while (de != NULL) {

        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
            de = readdir(dd);

            continue;
        }

        char name[PATH_MAX];
        sprintf(name, "%s/%s", dirname, de->d_name);

        if (de->d_type == 8) {
            
            if (sp == NULL) {
                goto ignore;
            }

            if (strlen(sp->name) != 0 && strstr(de->d_name, sp->name) == NULL) {
                goto jump;
            }

            if (strlen(sp->extension) != 0 && strstr(de->d_name, sp->extension) == NULL) {
                goto jump;
            }

            if (sp->size != 0) {
                int size = fileSize(name);
                // maximim size
                if (sp->size < 0 && size > -sp->size) {
                    goto jump;
                }
                // minimum size
                if (sp->size > 0 && size < sp->size) {
                    goto jump;
                }
            }

            ignore:

            sprintf(files[(*n)].path, "%s", name);
            sprintf(files[(*n)].name, "%s", de->d_name);
            (*n)++;
        } else {
            MyFind(name, files, n, sp);
        }

        jump:

        de = readdir(dd);
    }

    closedir(dd);
}

void printColors() {
    printf(BLK "Lorem ipsum dolor sit amet\n" reset);
    printf(BRED "Lorem ipsum dolor sit amet\n" reset);
    printf(BGRN "Lorem ipsum dolor sit amet\n" reset);
    printf(BYEL "Lorem ipsum dolor sit amet\n" reset);
    printf(BBLU "Lorem ipsum dolor sit amet\n" reset);
    printf(BMAG "Lorem ipsum dolor sit amet\n" reset);
    printf(BCYN "Lorem ipsum dolor sit amet\n" reset);
    printf(BWHT "Lorem ipsum dolor sit amet\n" reset);

    // printf("\e[1;38m" "Lorem ipsum dolor sit amet\n" reset);
    // printf("\e[1;39m" "Lorem ipsum dolor sit amet\n" reset);
}