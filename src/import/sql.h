#pragma once

#include "sql_low.h"

struct User {
    int userID;
    char firstname[255];
    char lastname[255];
    char email[255];
    char password[255];
};
struct File {
    int fileID;
    int userID;
    char name[255];
    char path[4096];
};

typedef struct User User;
typedef struct File File;

void initializeDatabase(sqlite3*);

void insertUser(sqlite3*, User*);
void addFileToUser(sqlite3*, File*, User*);

void showAllUsers(sqlite3* db);
User getUserByEmail(sqlite3* db, char*);

File getFile(sqlite3*, int, char*);
void getUserFiles(sqlite3*, User*, File*, int*);

void showUser(User*);
void showFile(File*);

