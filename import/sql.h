#pragma once

#include "sql_low.h"

struct User {
    int userID;
    char* firstname;
    char* lastname;
    char* email;
    char* password;
};
struct File {
    int fileID;
    char* userID;
    char* name;
    char* path;
};

typedef struct User User;
typedef struct File File;

void initializeDatabase(sqlite3*);

void insertUser(sqlite3*, User*);
void insertFile(sqlite3*, File*);

void showAllUsers(sqlite3* db);
User getUserByEmail(sqlite3* db, char*);