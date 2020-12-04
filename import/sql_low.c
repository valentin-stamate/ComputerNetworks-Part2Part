#include "./sql_low.h"
#include "sqlite/sqlite3.h"
#include <stdio.h>
#include <stdlib.h>

sqlite3* openDatabase(char* databaseName) {
    sqlite3* db;

    sqlite3_open(databaseName, &db);

    return db;
}
void closeDatabase(sqlite3* db) {
    sqlite3_close(db);
}

// This will stop the program if the execution fails
void SQLExecute(sqlite3* db, char* sql) {
    char *error;
    int res = sqlite3_exec(db, sql, NULL, NULL, &error);

    if (res != SQLITE_OK) {
        printf("%s\n", error);
        exit(0);
    }
}

void SQLInsert(sqlite3* db, char* sql) {
    
    char *error;
    int res = sqlite3_exec(db, sql, NULL, NULL, &error);

    if (res != SQLITE_OK) {
        printf("%s\n", error);
    }
}



void SQLGet(sqlite3* db, char* sql, int* nRow, int* nCol, char result[100][10][100]) { // taken from https://stackoverflow.com/questions/3957343/how-to-read-data-from-sqlite-database
    sqlite3_stmt* stmt = NULL;

    int retval, idx;

    retval = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if(retval != SQLITE_OK) {
        *nRow = -1;
        printf("Selecting data from DB Failed (err_code=%d)\n", retval);
        return;
    }

    *nRow = 0;
    *nCol = 0;

    int done = 0;
    while(!done) {
        
        retval = sqlite3_step(stmt);

        switch (retval) {
        case SQLITE_ROW:
            *nCol = sqlite3_column_count(stmt);

            for (int i = 0; i < *nCol; i++) {
                sprintf(result[*nRow][i], "%s", sqlite3_column_text(stmt, i));
            }
         
            (*nRow)++;

            break;
        case SQLITE_DONE:
            done = 1;
            break;
        default:
            printf("Some error encountered\n");
            done = 1;
            break;
        }
      
    }

    sqlite3_finalize(stmt);

}

void SQLUpdate(sqlite3* db, char* sql) {

}
void SQLDelete(sqlite3* db, char* sql) {

}