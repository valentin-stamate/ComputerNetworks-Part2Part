#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "utility.h"

char *getIp() {
    char cmd[100];
    sprintf(cmd, "%s", "ifconfig | grep inet | grep 192 | xargs | cut -d' ' -f2");

    FILE *fdRet = popen(cmd, "r");

    char line[4096];

    char *text = (char*)malloc(100);
    bzero(text, 100);

    while (fgets(line, 4096, fdRet) != NULL) {
        strcat(text, line);
    }

    text[strlen(text) - 1] = '\0';

    return text;
}