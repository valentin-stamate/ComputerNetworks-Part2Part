#pragma once

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

#include <pthread.h>

#include "server/server_functions.h"
#include "user_commands.h"
#include "errors.h"
#include "utility.h"

#define MAX_FILES 100

#define DEFAULT_GATEWAY_IP "192.168.1.9"
#define DEFAULT_PORT 1024