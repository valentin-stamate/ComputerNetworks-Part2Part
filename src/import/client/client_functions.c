#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "client_functions.h"
#include "../ANSI-color-codes.h"
#include "../user_commands.h"
#include "../errors.h"

void showWelcomeMessage() {
    printf(BLU "Welcome to Part2Part.\nIn order to run rommands log in fist using login <email> <password>\n\n" reset);
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

    if ( blocks == 3 && strcmp(command[0], "login") == 0) {
        return LOGIN;
    }

    if ( blocks == 1 && strcmp(command[0], "logout") == 0) {
        return LOGOUT;
    }

    if (blocks == 1 && strcmp(command[0], "quit") == 0) {
        printf("Process killed\n");
        exit(1);
    }

    return ERROR;
}

int sendLoginCredentials(int sd, char command[10][100], User* u) {

    int type = LOGIN;
    if (write(sd, &type, sizeof(int)) == -1) {
        printf("[LOGIN 1] " WRITE_ERROR "\n");
        return -1;
    }
    
    sprintf(u->email, "%s", command[1]);
    sprintf(u->password, "%s", command[2]);

    if (write(sd, u, sizeof(User)) == -1) {
        printf("[LOGIN 2] " WRITE_ERROR "\n");
        return -1;
    }

    if (read(sd, u, sizeof(User)) == -1) {
        printf("[LOGIN] " READ_ERROR "\n");
        return -1;
    }
    
    return u->userID != -1;

}


