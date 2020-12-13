#pragma once

#include <stdio.h>

struct User {
    int userID;
    char username[255];
    char email[255];
    char password[255];

    int isActive;
};
struct File {
    int fileID;
    int userID;
    char name[255];
    char path[1024];
};

typedef struct User User;
typedef struct File File;

void showUser(User*);

void showFile(File*);