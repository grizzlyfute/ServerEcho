#define _XOPEN_SOURCE 700

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h> /* getprotobyname */
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	char buffer[BUFSIZ];
	char protoname[] = "tcp";
	struct protoent *protoent;
	char *server_hostname = NULL;
	in_addr_t in_addr;
	// in_addr_t server_addr;
	int sockfd;
	ssize_t nbytes, ntot, inlen;
	struct hostent *hostent;
	/* This is the struct used by INet addresses. */
	struct sockaddr_in sockaddr_in;
	unsigned short server_port = 0;

	if (argc > 2)
	{
		server_hostname = argv[1];
		server_port = strtol(argv[2], NULL, 10);
	}
	else
	{
		printf("usage %s serveur port\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Get socket. */
	protoent = getprotobyname(protoname);
	if (protoent == NULL)
	{
		perror("getprotobyname");
		exit(EXIT_FAILURE);
	}

	/* Prepare sockaddr_in. */
	hostent = gethostbyname(server_hostname);
	if (hostent == NULL)
	{
		fprintf(stderr, "error: gethostbyname(\"%s\")\n", server_hostname);
		exit(EXIT_FAILURE);
	}
	in_addr = inet_addr(inet_ntoa(*(struct in_addr*)*(hostent->h_addr_list)));
	if (in_addr == (in_addr_t)-1)
	{
		fprintf(stderr, "error: inet_addr(\"%s\")\n", *(hostent->h_addr_list));
		exit(EXIT_FAILURE);
	}
	sockaddr_in.sin_addr.s_addr = in_addr;
	sockaddr_in.sin_family = AF_INET;
	sockaddr_in.sin_port = htons(server_port);

	while (1)
	{
		sockfd = socket(AF_INET, SOCK_STREAM, protoent->p_proto);
		if (sockfd == -1)
		{
			perror("socket");
			exit(EXIT_FAILURE);
		}
		if (connect(sockfd, (struct sockaddr*)&sockaddr_in, sizeof(sockaddr_in)) < 0)
		{
			perror("connect");
			exit(EXIT_FAILURE);
		}

		// Read data
		if (fgets(buffer, BUFSIZ, stdin) == NULL)
		{
			perror("fgets");
			exit(EXIT_FAILURE);
		}
		for (inlen = 0; buffer[inlen]; inlen++) {}
		printf ("Read %ld bytes\n", inlen);

		ntot = 0;
		while (ntot < inlen)
		{
			nbytes = write(sockfd, &buffer[ntot], inlen - ntot);
			if (nbytes <= 0)
			{
				perror("write");
				exit(EXIT_FAILURE);
			}
			ntot += nbytes;
		}
		printf("Send %ld bytes of data\n", ntot);

		ntot = 0;
		while (ntot < inlen)
		{
			nbytes = read(sockfd, &buffer[ntot], inlen - ntot);
			if (nbytes < 0)
			{
				perror("read");
				exit(EXIT_FAILURE);
			}
			ntot += nbytes;
		}

		printf("Server responds %ld bytes: { ", ntot);
		// print byte has hex
		for (nbytes = 0; nbytes < ntot; nbytes++)
		{
			printf(" 0x%02x,", (unsigned char)buffer[nbytes]);
		}
		printf(" }\n");
		fflush(stdout);

		close (sockfd);
	}

	exit(EXIT_SUCCESS);
}
