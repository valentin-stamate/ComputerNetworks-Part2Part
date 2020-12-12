#include "import/client_static.h"

extern int errno;

int isLogged = 0;

User *user;
File user_files[100];

int main(int argc, char *argv[]) {

    int sd;			
    struct sockaddr_in server;	


    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(GATEWAY_IP);
    server.sin_port = htons(PORT);
    
    // 

    user = (User*) malloc(sizeof(User));
    user->userID = -1;

    char rawCommand[100];
    char SIGGNED_AS[300] = "";

    system("clear");

    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        perror (SOCKET_ERROR);
        return errno;
    }

    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1) {
        perror (CONNECT_ERROR);
        return errno;
    }

    repeat:
    
    showWelcomeMessage(user);
    
    sprintf(SIGGNED_AS, "%s", "");
    if (isLogged == 1) {
        sprintf(SIGGNED_AS, BBLU "[" BGRN "%s" BBLU "]" reset, user->username);
    }

    printf("%s" CLI " ", SIGGNED_AS);
    getLine(rawCommand, sizeof(rawCommand));

    trimString(rawCommand, ' ');

    char command[10][100];
    int blocks = 0;

    getBlocks(command, rawCommand, &blocks);

    int COMMAND_TYPE = process(command, blocks);
    
    switch (COMMAND_TYPE) {
    case LOGIN:

        sendLoginCredentials(sd, command, user);

        isLogged = (user->userID != -1);
    
        if (isLogged == 1) {
            printf("Logged in. Welcome " BGRN "%s.\n\n" reset, user->username);
        } else {
            printf("Invalid credentials\n\n");
        }

        break;
    case SIGNUP:
       
        getUserCredentials(sd, user);

        isLogged = (user->userID != -1);

        if (isLogged == 1) {
            printf("Successfully signed in. Welcome " BCYN "%s.\n\n" reset, user->username);
        } else {
            printf("Invalid credentials. User may already exist.\n\n");
        }

        break;
    case LOGOUT:
        printf("Logged out\n\n");

        isLogged = 0;
        user->userID = -1;

        int r = LOGOUT;
        if (write(sd, &r, sizeof(int)) == -1) {
            printf("[LOGOUT] " WRITE_ERROR "\n");
        }

        break;

    default:
        printf("Invalid command! To see all commands press " BWHT "help.\n\n" reset);
        break;
    }

    // int nr=0;
    // char buf[10];

    // printf ("[client]Introduceti un numar: ");
    // fflush (stdout);
    // read (0, buf, sizeof(buf));
    // nr=atoi(buf);
    
    // printf("[client] Am citit %d\n",nr);

    // if (write (sd,&nr,sizeof(int)) <= 0){
    //     perror (WRITE_TO_SERVER_ERROR);
    //     return errno;
    // }

    // if (read (sd, &nr,sizeof(int)) < 0) {
    //     perror (READ_TO_SERVER_ERROR);
    //     return errno;
    // }

    system("clear");

    goto repeat;

    close (sd);

    return 0;
}






