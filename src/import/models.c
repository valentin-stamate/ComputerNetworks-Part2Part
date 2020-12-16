#include <stdio.h>
#include "models.h"

void showUser(User* u) {
    printf("%d %s %s %s\n", u->userID, u->username, u->email, u->password);
}