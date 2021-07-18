/* Simple TCP echo server
 * gcc -Wall -Wextra -O3 tcp_echo.c -o efbServerEcho
 * usage: tcpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include <sys/wait.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFSIZE 1024

/*
 * error - wrapper for perror
 */
void error(char *msg)
{
  perror(msg);
  exit(1);
}

int main(int argc, char **argv)
{
  int parentfd; /* parent socket */
  int childfd; /* child socket */
  int portno; /* port to listen on */
  socklen_t clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buffer */
  char *hostaddrp = NULL; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */

  // avoid defunct process (see also waitpid):
  signal(SIGCHLD, SIG_IGN);

  tzset();

  /*
   * check command line arguments
   */
  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /*
   * socket: create the parent socket
   */
  parentfd = socket(AF_INET, SOCK_STREAM, 0);
  if (parentfd < 0)
  {
    error("ERROR opening socket");
  }

  /* setsockopt: Handy debugging trick that lets
   * us rerun the server immediately after we kill it;
   * otherwise we have to wait about 20 secs.
   * Eliminates "ERROR on binding: Address already in use" error.
   */
  optval = 1;
  setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR,
       (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));

  /* this is an Internet address */
  serveraddr.sin_family = AF_INET;

  /* let the system figure out our IP address */
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

  /* this is the port we will listen on */
  serveraddr.sin_port = htons((unsigned short)portno);

  /*
   * bind: associate the parent socket with a port
   */
  if (bind(parentfd, (struct sockaddr *) &serveraddr,
     sizeof(serveraddr)) < 0)
  {
    error("ERROR on binding");
  }

  /*
   * listen: make this socket ready to accept connection requests
   */
  if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */
  {
    error("ERROR on listen");
  }

  /*
   * main loop: wait for a connection request, echo input line,
   * then close connection.
   */
  clientlen = sizeof(clientaddr);
  while (1)
  {

    /*
     * accept: wait for a connection request
     */
    childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
    if (childfd < 0)
    {
      perror("ERROR on accept");
      continue;
    }

    if (fork() == 0)
    {
      // I am the child

      /*
       * gethostbyaddr: determine who sent the message
       */
      hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
      if (hostp == NULL)
      {
        perror("ERROR on gethostbyaddr");
      }
      else
      {
        hostaddrp = inet_ntoa(clientaddr.sin_addr);
      }

      if (hostaddrp == NULL)
      {
        perror("ERROR on inet_ntoa");
      }
      else
      {
        printf("Server established connection with %s (%s)\n", hostp->h_name, hostaddrp);
      }

      fd_set rfds;
      struct timeval tv;
      short isOk = 1;
      time_t timestamp;
      int i;
      timestamp = time(NULL);
      printf("(%zu) %s\n", timestamp, asctime(localtime((time_t*)&timestamp)));
      while (isOk)
      {
        FD_ZERO(&rfds);
        FD_SET(childfd, &rfds);

        /* Attends jusqu'à 7 secondes. */
        tv.tv_sec = 7;
        tv.tv_usec = 0;

        n = select(childfd+1, &rfds, NULL, NULL, &tv);
        /* tv undefined */

        if (n < 0)
        {
          perror("select()");
          isOk = 0;
          continue;
        }
        else if (n == 0)
        {
          printf ("timeout\n");
          isOk = 0;
          continue;
        }

        /*
         * read: read input string from the client
         */
        bzero(buf, BUFSIZE);
        n = read(childfd, buf, BUFSIZE);
        if (n <= 0)
        {
          perror("ERROR reading from socket");
          isOk = 0;
        }
        else
        {
          timestamp = time(NULL);
          printf("(%zu) %s server received %d bytes:\n {",
              timestamp, asctime(localtime((time_t*)&timestamp)), n);

          // print byte has hex
          for (i = 0; i < n; i++)
          {
            printf(" 0x%02x,", (unsigned char)buf[i]);
          }
          printf(" }\n");
        }

        /*
         * write: echo the input string back to the client
         */
        n = write(childfd, buf, n);
        if (n < 0)
        {
          perror("ERROR writing to socket");
          isOk = 0;
        }
      }
      printf("Child terminated\n");

      close(childfd);
      close(parentfd);
      exit(0);
    }
    else
    {
      // I am the father
      close(childfd);
    }
  }
}

