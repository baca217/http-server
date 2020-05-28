#include <stdio.h>
#include <stdlib.h>
#include <string.h>      /* for fgets */
#include <strings.h>     /* for bzero, bcopy */
#include <unistd.h>      /* for read, write */
#include <sys/socket.h>  /* for socket use */
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAXLINE  8192  /* max text line length */
#define MAXBUF   8192  /* max I/O buffer size */
#define LISTENQ  1024  /* second argument to listen() */
#define SENDPORT 4000

/***************************************************************************************************
 * For now, the message is not being sent to the server side. This is maybe because of how I'm binding
 * the server address. Maybe I need to check the port number being used haha
 *
 * Also, look into the connect and send functions. This is FTP we're talking about don't be a dumbass.
 * This requires a few more protocols than UDP.
 * ************************************************************************************************/
void echo(int connfd);

int main(int argc, char** argv)
{
	int port = 4001, serv_len=sizeof(struct sockaddr_in), sockfd;
        struct sockaddr_in serv_addr;
	char msg[] = "HTTP/1.1 200 Document Follows\r\nContent-Type:text/html\r\nContent-Length:32\r\n\r\n<html><h1>Hello CSCI4273 Course!</h1>";

        printf("using port %d for now\n", port);

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("ERROR: couldn't bind socket");
		return -1;
	}

	memset(&serv_addr, '0', serv_len);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SENDPORT);
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(sockfd, (struct sockaddr *)&serv_addr, serv_len) < 0)
	{
		printf("ERROR: Connect Failed\n");
		return -1;
	}

	if(send(sockfd, msg, strlen(msg), 0) < 0)
	{
		printf("ERROR: message send failed");
		return -1;
	}
	
        return 1;
}
