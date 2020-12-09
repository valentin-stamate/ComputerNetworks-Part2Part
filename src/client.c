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

#define MAX_FILES 100

#define GATEWAY_IP "192.168.1.9"
#define PORT 2024

extern int errno;

int main(int argc, char *argv[]) {

    int sd;			
    struct sockaddr_in server;	
  		
    int nr=0;
    char buf[10];

    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        perror ("Eroare la socket().\n");
        return errno;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(GATEWAY_IP);
    server.sin_port = htons(PORT);
    
    
    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1) {
        perror ("[client]Eroare la connect().\n");
        return errno;
    }

    printf ("[client]Introduceti un numar: ");
    fflush (stdout);
    read (0, buf, sizeof(buf));
    nr=atoi(buf);
    
    printf("[client] Am citit %d\n",nr);

    if (write (sd,&nr,sizeof(int)) <= 0){
        perror ("[client]Eroare la write() spre server.\n");
        return errno;
    }

    if (read (sd, &nr,sizeof(int)) < 0) {
        perror ("[client]Eroare la read() de la server.\n");
        return errno;
    }
    
    printf ("[client]Mesajul primit este: %d\n", nr);


    close (sd);

    return 0;
}






