#include "import/client_static.h"

extern int errno;

int isLogged = 0;

User *user;
File user_files[100];

int main(int argc, char *argv[]) {

    int sd;			
    struct sockaddr_in server;	

    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        perror (SOCKET_ERROR);
        return errno;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(GATEWAY_IP);
    server.sin_port = htons(PORT);
    
    
    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1) {
        perror (CONNECT_ERROR);
        return errno;
    }

    user = (User*) malloc(sizeof(User));

    // 
    showWelcomeMessage();

    char rawCommand[100];

    repeat:

    printf(CLI " ");
    fgets(rawCommand, sizeof(rawCommand), stdin);
    rawCommand[(int)strlen(rawCommand) - 1] = '\0'; // removing newline

    trimString(rawCommand, ' ');

    char command[10][100];
    int blocks = 0;

    getBlocks(command, rawCommand, &blocks);

    int COMMAND_TYPE = process(command, blocks);

    switch (COMMAND_TYPE) {
    case LOGIN:
        printf("Loggin in...\n");

        sendLoginCredentials(sd, command, user);

        showUser(user);

        isLogged = (user->userID != -1);
    
        if (isLogged == 1) {
            printf("Logged in. Welcome " BGRN "%s.\n\n" reset, user->firstname);
        } else {
            printf("Invalid credentials\n\n");
        }


        break;
    
    case LOGOUT:
        printf("Logged out\n\n");
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

    goto repeat;

    close (sd);

    return 0;
}






