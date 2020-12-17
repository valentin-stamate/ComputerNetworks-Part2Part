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
    char name[255];
    char path[1024];
};

struct RequestedFile {
    int user_id;
    char username[255];
    char fileName[255];
    char filePath[1024];
};

struct SearchFile {
    int user_id;
    char params[255];
};

struct SearchParams {
    char name[255];         // the (sub)name required
    char extension[10];     // the extension eg. .txt .mkv etc
    int size;               // the (-)minimum/(+)maximum size a file can have
};



typedef struct User User;
typedef struct File File;
typedef struct RequestedFile RequestedFile;
typedef struct SearchFile SearchFile;
typedef struct SearchParams SearchParams;

void showUser(User*);

void showFile(File*);