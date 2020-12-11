#include <stdio.h>
#include "models.h"

void showUser(User* u) {
    printf("%d %s %s %s\n", u->userID, u->username, u->email, u->password);
}

void showFile(File* f) {
    printf("%d, %d, %s, %s\n", f->fileID, f->userID, f->name, f->path);
}