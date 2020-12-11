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
    
    printf("Email: ");
    getLine(u->email, 255);
    printf("Password: ");
    getLine(u->password, 255);

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

void getLine(char* buffer, int n) {
    fgets(buffer, n, stdin);
    buffer[(int)strlen(buffer) - 1] = '\0'; // removing newline
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