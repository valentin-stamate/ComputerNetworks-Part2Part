/* cliTcpConc.c - Exemplu de client TCP
   Trimite un nume la server; primeste de la server "Hello nume".
         
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <sys/socket.h>
/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  char msg[100];		// mesajul trimis

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }

  int type = 1;

  // scanf("%d", &type);

  char fileName[50];
  strcpy(fileName, "");

  int clientNumber;
  read(sd, &clientNumber, 4);

  sprintf(fileName, "donloaded_%d.txt", clientNumber);

  printf("File is %s\n", fileName);

  write(sd, &type, 4);

  if (type == 1) {
    printf("Client is requesting to download the file\n");

    if (access(fileName, F_OK) == -1) {
        int fd = open(fileName, O_RDWR | O_CREAT);

        if (fd == -1) {
            error("Errrofs");
        }

        if ( close(fd) == -1) {
            error("Error");
        }
    }

    FILE* fileToStore = fopen(fileName, "a");
  
    if (fileToStore == NULL) {
      perror("Open Error");
    }

    int bytesToRead;

    while (1) {
      read(sd, &bytesToRead, 4);

      if (bytesToRead == -1) {
        break;
      }

      char line[4096];
      read(sd, line, bytesToRead);


      fprintf(fileToStore, "%s", line);

    } 

    fclose(fileToStore);


    // close(fd);
  } else {
    printf("Nothing\n");
  }

  close (sd);
}