#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "client_functions.h"
#include "../ANSI-color-codes.h"
#include "../user_commands.h"
#include "../errors.h"

void showWelcomeMessage(User* u) {
    char user[300] = "";
    if (u->userID != -1) {
        sprintf(user, "%s ", u->username);
    }
    printf(BBLU "Welcome " BGRN "%s" BBLU "to Part2Part.\nIn order to run rommands log in fist using login\n\n" reset, user);
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

void getBlocks(char destination[10][100], char *source, int *len) {
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

int process(char command[10][100], int blocks) {
    if ( blocks == 1 && strcmp(command[0], "help" ) == 0) {
        return HELP;
    }

    if ( blocks == 1 && strcmp(command[0], "login") == 0) {
        return LOGIN;
    }

    if ( blocks == 1 && strcmp(command[0], "signup") == 0 ) {
        return SIGNUP;
    }

    if ( blocks == 1 && strcmp(command[0], "logout") == 0) {
        return LOGOUT;
    }

    if ( blocks == 2 && strcmp(command[0], "show") == 0 && strcmp(command[1], "users") == 0 ) {
        return GET_USERS;
    }

    if (blocks == 1 && strcmp(command[0], "quit") == 0) {
        printf("Process killed\n");
        exit(1);
    }

    return ERROR;
}

void sendLoginCredentials(int sd, char command[10][100], User* u) {
    
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

void getUsers(int sd, char notification[10][100], int* n) {
    int type = GET_USERS;
    if (write(sd, &type, sizeof(int)) == -1) {
        printf("[LOGIN 1] " WRITE_ERROR "\n");
        return;
    }

    int nUsers;

    while ((*n) > 0) {
        popNotification(notification, n);
    }

    read(sd, &nUsers, sizeof(int));

    for (int i = 0; i < nUsers; i++) {
        User u;
        read(sd, &u, sizeof(User));

        char* line = malloc(100);

        if (u.isActive == 1) {
            sprintf(line, BWHT "%s" reset " with id " BWHT "%d" reset " is " BGRN "active" reset, u.username, u.userID);
        } else {
             sprintf(line, BWHT "%s" reset " with id " BWHT "%d" reset " is " BYEL "offline" reset, u.username, u.userID);
        }

        pushNotification(line, notification, n);

        free(line);
    }

    char* line = malloc(100);

    if (nUsers != 0) {
        sprintf(line, "To connect with a user type the command connect to <user_id>");

        pushNotification(line, notification, n);

    } else {
        sprintf(line, "No active users found");

        pushNotification(line, notification, n);
    }
}

void getLine(char* buffer, int n) {
    fgets(buffer, n, stdin);
    buffer[(int)strlen(buffer) - 1] = '\0'; // removing newline
}

void showNotifications(char notification[10][100], int n) {
    for (int i = 0; i < n; i++) {
        showNotification(notification[i]);
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
        printf("=  %s  =\n", rowText[i]);
    }

    printf("=  %s", rowText[nRows - 1]);
    int r = strlen(rowText[nRows - 1]);
    r = cut - r + 2;

    for (int i = 1; i <= r; i++) {
        printf(" ");
    }
    printf("=\n");
    printf("===========================================================================\n");


}

void pushNotification(char* newNot, char notifications[10][100], int* n) {
    sprintf(notifications[(*n)++], "%s", newNot);
}

void popNotification(char notifications[10][100], int* n) {
    for (int i = 0; i < 9; i++) {
        sprintf(notifications[i], "%s", notifications[i + 1]);
    }
    sprintf(notifications[9], "%s", "");
    (*n) = (*n) - 1;
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