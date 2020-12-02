#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <netdb.h>
#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "sqlite/sqlite3.h"

#define DATABASE "data.db"

sqlite3* openDatabase(char*);

void SQLInsert(sqlite3*, char*);
void SQLGet(sqlite3*, char*);
void SQLUpdate(sqlite3*, char*);
void SQLDelete(sqlite3*, char*);

int main() {

    sqlite3 *db = openDatabase(DATABASE);

    char *error;
    int res = sqlite3_exec(db, "CREATE TABLE contacts (contact_id INTEGER PRIMARY KEY,first_name TEXT NOT NULL,last_name TEXT NOT NULL,email TEXT NOT NULL UNIQUE,phone TEXT NOT NULL UNIQUE);", NULL, NULL, &error);

    return 0;

}

sqlite3* openDatabase(char* databaseName) {
    sqlite3* db;

    sqlite3_open(databaseName, &db);

    return db;
}