#include <stdio.h>
#include "sql.h"
#include <string.h>
#include <stdlib.h>

void initializeDatabase(sqlite3* db) {
    SQLExecute(db, "CREATE TABLE users( id INTEGER PRIMARY KEY, firstname VARCHAR(255) NOT NULL, lastname VARCHAR(255) NOT NULL, email VARCHAR(255) UNIQUE NOT NULL, password TEXT NOT NULL);");
    SQLExecute(db, "CREATE TABLE files(id INTEGER PRIMARY KEY, user_id INTEGER, name VARCHAR(255) NOT NULL, path TEXT, FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE ON UPDATE NO ACTION);");

    User u = {1, "Valentin", "Stamate", "stamatevalentin125@gmail.com", "23456789"};

    insertUser(db, &u);

    File f = {0, 0, "fisier.txt", "desktop/fisier.txt"}; 
    addFileToUser(db, &f, &u);

    sprintf(f.name, "fisier_2.txt");
    sprintf(f.path, "desktop/fisier_2.txt");

    addFileToUser(db, &f, &u);

}

void insertUser(sqlite3* db, User* user) {
    char sql[1250];

    sprintf(sql, "INSERT INTO users(firstname, lastname, email, password) VALUES ('%s', '%s', '%s', '%s');", user->firstname, user->lastname, user->email, user->password);

    SQLInsert(db, sql);

    User u = getUserByEmail(db, user->email);

    user->userID = u.userID;

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
    strcpy(u.firstname, result[0][1]);
    strcpy(u.lastname, result[0][2]);
    strcpy(u.email, result[0][3]);
    strcpy(u.password, result[0][4]);

    return u;
}

void addFileToUser(sqlite3* db, File* file, User* user) {
    char sql[5000];

    sprintf(sql, "INSERT INTO files(user_id, name, path) VALUES(%d, '%s', '%s');", user->userID, file->name, file->path);

    SQLInsert(db, sql);

    file->userID = user->userID;

    File f = getFile(db, user->userID, file->path); // i can't have the same file twice for the same user

    file->fileID = f.fileID;

}

File getFile(sqlite3* db, int user_id, char* file_path) {
    int n, m;
    char result[1][10][100];

    char sql[500];

    sprintf(sql, "SELECT * FROM files WHERE user_id = %d AND path = '%s';", user_id, file_path);

    SQLGet(db, sql, &n, &m, result);

    File f;

    if (n == 0) {
        f.fileID = -1;
        return f;
    }

    f.fileID = atoi(result[0][0]);
    f.userID = atoi(result[0][1]);
    strcpy(f.name, result[0][2]);
    strcpy(f.path, result[0][3]);
    
    return f;
}

void getUserFiles(sqlite3* db, User* user, File* files, int* nFile) {
    int n, m;
    char result[100][10][100];

    char sql[1250];
    sprintf(sql, "SELECT * FROM files WHERE user_id = %d;", user->userID);

    SQLGet(db, sql, &n, &m, result);

    *nFile = n;

    for (int i = 0; i < n; i++) {
        files[i].fileID = atoi(result[i][0]);
        files[i].userID = atoi(result[i][1]);
        strcpy(files[i].name, result[i][2]);
        strcpy(files[i].path, result[i][3]);
    }
}
