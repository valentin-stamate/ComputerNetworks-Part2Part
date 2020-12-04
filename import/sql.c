#include "stdio.h"
#include "sql.h"
#include <string.h>
#include <stdlib.h>

void initializeDatabase(sqlite3* db) {
    SQLExecute(db, "CREATE TABLE user( id INTEGER PRIMARY KEY, firstname VARCHAR(255) NOT NULL, lastname VARCHAR(255) NOT NULL, email VARCHAR(255) UNIQUE NOT NULL, password TEXT NOT NULL);");

    User u;

    u.firstname = "Valentin";
    u.lastname = "Stamate";
    u.password = "23456789";
    u.email = "stamatevalentin125@gmail.com";

    insertUser(db, &u);
}

void insertUser(sqlite3* db, User* user) {
    char sql[1000];

    sprintf(sql, "INSERT INTO user(firstname, lastname, email, password) VALUES ('%s', '%s', '%s', '%s');", user->firstname, user->lastname, user->email, user->password);

    SQLInsert(db, sql);
}

void showAllUsers(sqlite3* db) {
    int n, m;
    char result[100][10][100];

    SQLGet(db, "SELECT * FROM user;", &n, &m, result);

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

    sprintf(sql, "SELECT * FROM user WHERE email = '%s';", email);

    SQLGet(db, sql, &n, &m, result);

    User u;

    if (n == 0) {
        u.userID = -1;
        return u;
    }

    u.userID = atoi(result[0][0]);
    u.firstname = result[0][1];
    u.lastname = result[0][2];
    u.email = result[0][3];
    u.password = result[0][4];
    
    return u;
}

void insertFile(sqlite3* db, File* file) {

}