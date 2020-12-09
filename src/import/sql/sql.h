#pragma once

#include "sql_low.h"
#include "../models.h"

void initializeDatabase(sqlite3*);

void insertUser(sqlite3*, User*);
void addFileToUser(sqlite3*, File*, User*);

void showAllUsers(sqlite3* db);
User getUserByEmail(sqlite3* db, char*);

File getFile(sqlite3*, int, char*);
void getUserFiles(sqlite3*, User*, File*, int*);

