#pragma once

#include <stddef.h>
#include "../sqlite/sqlite3.h"

#define DATABASE "data.db"

sqlite3* openDatabase(char*);
void closeDatabase(sqlite3*);
void SQLExecute(sqlite3*, char*);

int SQLInsert(sqlite3*, char*);
void SQLGet(sqlite3*, char*, int*, int*, char[100][10][100]);
void SQLUpdate(sqlite3*, char*);
void SQLDelete(sqlite3*, char*);