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

#include "import/sql.h"

#define MAX_FILES 100

int main(int argc, char *argv[]) {

    sqlite3 *db = openDatabase(DATABASE);

    if (argc == 2) {
        initializeDatabase(db);
    }
    
    User u;
    File file[MAX_FILES];
    int nFiles = 0;

    char *email = "stamatevalentin125@gmail.com";

    u = getUserByEmail(db, email);
    showUser(&u);

    getUserFiles(db, &u, file, &nFiles);

    for (int i = 0; i < nFiles; i++) {
        showFile(file + i);
    }

    closeDatabase(db);
    return 0;
}






