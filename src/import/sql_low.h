#pragma once

#include <stddef.h>
#include "sqlite/sqlite3.h"

#define DATABASE "data.db"
#define CREATE_USER_TABLE "CREATE TABLE IF NOT EXISTS user( \
    id INTEGER PRIMARY KEY AUTOINCREMENT, \
    firstname VARCHAR(255) NOT NULL, \
    lastname VARCHAR(255) NOT NULL, \
    email VARCHAR(255) NOT NULL UNIQUE, \
    password TEXT NOT NULL \
);"

#define CREATE_FILE_TABLE "CREATE TABLE "


sqlite3* openDatabase(char*);
void closeDatabase(sqlite3*);
void SQLExecute(sqlite3*, char*);

void SQLInsert(sqlite3*, char*);
void SQLGet(sqlite3*, char*, int*, int*, char[100][10][100]);
void SQLUpdate(sqlite3*, char*);
void SQLDelete(sqlite3*, char*);