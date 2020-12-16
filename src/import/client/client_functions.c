#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include "client_functions.h"
#include "../ANSI-color-codes.h"
#include "../user_commands.h"
#include "../errors.h"

void showWelcomeMessage(User* u) {
    char user[300] = "";
    if (u->userID != -1) {
        sprintf(user, "%s ", u->username);
    }
    printf(BBLU "Welcome " BGRN "%s" BBLU "to Part2Part.\n", user);

    if (u->userID == -1) {
        printf("In order to run commands log in fist.\n\n" reset);
    } else {
        printf("To see all the commands type " BWHT "help" BBLU ".\n\n" reset);
    }

    printf(BWHT "Few useful commands: " YELB " help " reset " " GRNB BWHT " login " reset "\n\n");

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

    if (blocks == 2 && strcmp(command[0], "clear") == 0 && strcmp(command[1], "notifications") == 0) {
        return CLEAR_NOTIFICATIONS;
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
        return PUT_FILE;
    }

    if (blocks == 1 && strcmp(command[0], "quit") == 0) {
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
            sprintf(line, BWHT "%s" reset " with id " BWHT "%d" reset " is " BGRN "active" reset, users[i].username, users[i].userID);
        } else {
            sprintf(line, BWHT "%s" reset " with id " BWHT "%d" reset " is " BYEL "offline" reset, users[i].username, users[i].userID);
        }

        pushNotification(line, notification, n);

        free(line);
    }

    char* line = malloc(500);

    if (*nUsers != 0) {
        sprintf(line, "To connect with a user type the command connect to <user_id>");

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
        printf("===========================================================================\n\n");
    }
}

void showNotification(char* s) {
    int cut = 75 - 6;
    int nRows = strlen(s + 1) / cut + 1;

    char rowText[10][75];

    for (int i = 0; i < nRows - 1; i++) {
        strcpy(rowText[i], "");

        strncat(rowText[i], s + i * cut, cut);

    }

    sprintf(rowText[nRows - 1], "%s", s + (nRows - 1) * cut);

    printf("===========================================================================\n");
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

void showFileStatus(struct stat st) {
    char perm[10];
    strcpy(perm, "");
    strcat(perm, "---------");
    if( S_IRUSR & st.st_mode )  perm[0]='r';
    if( S_IWUSR & st.st_mode )  perm[1]='w';
    if( S_IXUSR & st.st_mode )  perm[2]='x';
    if( S_IRGRP & st.st_mode )  perm[3]='r';
    if( S_IWGRP & st.st_mode )  perm[4]='w';
    if( S_IXGRP & st.st_mode )  perm[5]='x';
    if( S_IROTH & st.st_mode )  perm[6]='r';
    if( S_IWOTH & st.st_mode )  perm[7]='w';
    if( S_IXOTH & st.st_mode )  perm[8]='x';

    if (strcmp(perm, "---------") == 0) {
        printf("File doesn't exist\n");
        return;
    }

    printf("\nFile status:\n"); 
    printf("Total size: %d\n", (int)st.st_size);
    printf("Tile of last access: %d\n", (int)st.st_atim.tv_sec);
    printf("Time of last modification: %d\n", (int)st.st_mtim.tv_sec);
    printf("ID of device containing file: %d\n", (int)st.st_dev);

    printf("File permissions: %s\n", perm);

    printf("\n");
}

struct stat MyStat(char *path, int* status) {
    struct stat st;

    int acc = access(path, F_OK);

    if (acc == -1) {
        perror(ACCESS_ERROR);
    }

    (*status) = acc == 0;

    if ( stat(path, &st) == -1 ) {
        perror(STAT_ERROR);
    }

    return st;
}

void MyFind(char *dirname, File* files, int *n, char* search_param) {
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
            sprintf(files[(*n)].path, "%s", name);
            sprintf(files[(*n)].name, "%s", de->d_name);
            (*n)++;
        } else {
            MyFind(name, files, n, search_param);
        }

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