#include <stdio.h>
#include "server_functions.h"
#include <string.h>
#include <stdlib.h>
#include "../sqlite/sqlite3.h"

void initializeDatabase(sqlite3* db) {
    SQLExecute(db, "CREATE TABLE users( id INTEGER PRIMARY KEY, username VARCHAR(255) NOT NULL, email VARCHAR(255) UNIQUE NOT NULL, password TEXT NOT NULL);");
    
    User u = {1, "ValentinSt", "stamatevalentin125@gmail.com", "123456789"};

    addUser(db, &u);
}

void addUser(sqlite3* db, User* user) {
    char sql[1250];

    sprintf(sql, "INSERT INTO users(username, email, password) VALUES ('%s', '%s', '%s');", user->username, user->email, user->password);

    int r = SQLInsert(db, sql);
    
    if (r == -1) {
        user->userID = -1;
        return;
    }

    User u = getUserByEmail(db, user->email);

    (*user) = u;
}

void showAllUsers(sqlite3* db) {
    int n, m;
    char result[100][10][100];

    SQLGet(db, "SELECT * FROM users;", &n, &m, result);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            printf("%s ", result[i][j]);
        }
        printf("\n");
    }
}

User getUserByEmail(sqlite3* db, char* email) {
    int n, m;
    char result[1][10][100];

    char sql[500];

    sprintf(sql, "SELECT * FROM users WHERE email = '%s';", email);

    SQLGet(db, sql, &n, &m, result);

    User u;

    if (n == 0) {
        printf("User with email %s not found\n", email);
        u.userID = -1;
        return u;
    }

    
    u.userID = atoi(result[0][0]);
    strcpy(u.username, result[0][1]);
    strcpy(u.email, result[0][2]);
    strcpy(u.password, result[0][3]);

    return u;
}

void verifyUser(sqlite3* db, User* u) {

    User fu = getUserByEmail(db, u->email);

    u->userID = -1;

    if (strcmp(u->password, fu.password) == 0) {
        (*u) = fu;
        return;
    }
}
