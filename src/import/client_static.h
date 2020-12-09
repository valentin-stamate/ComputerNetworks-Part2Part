#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
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

#include "user_commands.h"
#include "client/client_functions.h"
#include "ANSI-color-codes.h"
#include "models.h"

#include "errors.h"

#define CLI "$"

// CONFIGURATION
#define MAX_FILES 100
#define GATEWAY_IP "192.168.1.9"
#define PORT 2024