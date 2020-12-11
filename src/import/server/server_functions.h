#pragma once

#include "server_low.h"
#include "../models.h"
#include "../sqlite/sqlite3.h"

void initializeDatabase(sqlite3*);

void addUser(sqlite3*, User*);
void addFileToUser(sqlite3*, File*, User*);

void showAllUsers(sqlite3* db);
User getUserByEmail(sqlite3* db, char*);
void verifyUser(sqlite3* db, User*);

File getFile(sqlite3*, int, char*);
void getUserFiles(sqlite3*, User*, File*, int*);

