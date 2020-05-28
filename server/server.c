/************************************************************************************************
 * Elmer Baca Holguin
 * Network System
 * Project 2: HTTP TCP server
 * *********************************************************************************************/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>      /* for fgets */
#include <strings.h>     /* for bzero, bcopy */
#include <unistd.h>      /* for read, write */
#include <sys/socket.h>  /* for socket use */
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/stat.h>

#define MAXLINE  8192  /* max text line length */
#define MAXBUF   8192  /* max I/O buffer size */
#define LISTENQ  1024  /* second argument to listen() */
#define ROOTDIR "www" /*root dir for machine*/
#define ERROR "HTTP/1.1 500 Internal Server Error"
#define V1MSG " 200 Document Follows\r\nContent-Type:\r\nContent-Length:\r\nConnection: Closed\r\n\r\n"
#define V0MSG " 200 Document Follows\r\nContent-Type:\r\nContent-Length:\r\n\r\n"
#define ROOT "/index.html"

int open_listenfd(int port);
int msg_work(int connfdp);
void* thread(void * vargp);

int main(int argc, char **argv)
{
	int port, listenfd, *connfd;
	socklen_t len=sizeof(struct sockaddr_in);
	struct sockaddr_in clientaddr;
	pthread_t tid;

	if(argc < 2)
	{
		printf("please enter a port number\n");
		exit(-1);
	}
	if((port = atoi(argv[1])) == 0)
	{
		printf("Please enter a number\n");
		exit(-1);
	}
	if(port < 5000 || port > 65535)
	{
		printf("Please enter a port number greater than 5000 and less than 65535\n");
		exit(-1);
	}
	printf("Using port %d\n", port);
	
	if((listenfd = open_listenfd(port)) < 0)
	{
		printf("failed to bind\n");
		return -1;
	}

	while(1)
	{
		connfd = malloc(sizeof(int));
		*connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &len);	
		pthread_create(&tid, NULL, thread, connfd);
	}
	return 1;
}

void* thread(void * vargp)
{
	int connfd = *((int *)vargp);
	pthread_detach(pthread_self());
	free(vargp);
	msg_work(connfd);
	close(connfd);
	return NULL;
}

int msg_work(int connfd)
{
	char *buf;
	char *msg;
	char *fpath;
	char cont_type[25];
	FILE *fp;
	int c_fsize, m_size;
	struct stat stats;
	char *tok;
	char meth[10], ver[10];
	char *uri;
	char par_buf[MAXLINE];

	read(connfd, par_buf, MAXLINE);
	par_buf[MAXLINE] = '\0';

	//setting the method
	if((tok = strtok(par_buf, " ")) == NULL || strlen(tok) > 10)
	{
		printf("tok is empty or bigger than 10!!!(tok)\n");
		return -1;
	}
	strcpy(meth, tok);

	//getting the uri
	if((tok = strtok(NULL, " ")) == NULL)
	{
		printf("tok is empty!!!(uri)\n");
		return -1;
	}
	if(strcmp(tok, "/") == 0)
	{
		uri = malloc(strlen(ROOT));
		bzero(uri, strlen(ROOT));
		strcpy(uri, ROOT);
	}
	else
	{
		uri = malloc(strlen(tok) + 1);
		bzero(uri,strlen(tok));
		strcpy(uri, tok);
	}
	//getting the http version
	if((tok = strtok(NULL, "\n")) == NULL || strlen(tok) > 10)
	{
		printf("tok is empty or bigger than 10!!!(ver)\n");
		free(uri);
		return -1;
	}	
	if(strstr(tok, "HTTP/1.1") != NULL)
		strcpy(ver, "HTTP/1.1");
	else if(strstr(tok, "HTTP/1.0") != NULL)
		strcpy(ver, "HTTP/1.0");
	else
	{
		printf("meth doesn't match\n");
		return -1;

	}

		
	//determening the content type ex: html or js
	if(strstr(uri, ".html") != NULL)
		strcpy(cont_type ,"text/html");
	else if(strstr(uri, ".txt") != NULL)
		strcpy(cont_type ,"text/plain");
	else if(strstr(uri, ".png") != NULL)
		strcpy(cont_type ,"image/png");
	else if(strstr(uri, ".gif") != NULL)
		strcpy(cont_type ,"image/gif");
	else if(strstr(uri, ".jpg") != NULL)
		strcpy(cont_type ,"image/jpg");
	else if(strstr(uri, ".css") != NULL)
		strcpy(cont_type ,"text/css");
	else if(strstr(uri, ".js") != NULL)
		strcpy(cont_type ,"application/javascript");
	else
		strcpy(cont_type, "\0");
	if(cont_type == NULL)
	{
		printf("file extension couldn't be extracted\n");
		buf = malloc(strlen(ERROR) + 1);
		bzero(buf, strlen(ERROR) + 1);
		sprintf(buf, "%s", ERROR);
		write(connfd, buf, strlen(ERROR));
		free(uri);
		free(buf);
		return -1;
	}
	fpath = malloc(strlen(ROOTDIR) + strlen(uri) + 2);
	bzero(fpath, strlen(ROOTDIR) + strlen(uri) + 2);
	strcpy(fpath, ROOTDIR);
	strcat(fpath, uri);
	if((fp = fopen(fpath, "r")) == NULL)
	{
		printf("file %s failed to open\n", fpath);
		buf = malloc(strlen(ERROR) + 1);
		bzero(buf, strlen(ERROR) + 1);
		sprintf(buf, "%s", ERROR);
		write(connfd, buf, strlen(ERROR));
		free(uri);
		free(fpath);
		free(buf);
		return -1;
	}
	stat(fpath, &stats);
	//getting the size of the file
	c_fsize = floor(log10(abs(stats.st_size))) + 1;
	//setting the buffer with file contents
	buf = (char *)malloc(stats.st_size);
	bzero(buf, stats.st_size);
	fread(buf, stats.st_size, 1, fp);
	//allocating and setting the http message
	if((strcmp(ver, "HTTP/1.1")) == 0)
	{
		m_size = strlen(V1MSG) + strlen(ver) + strlen(cont_type) + c_fsize + stats.st_size + 50;
		msg = malloc(m_size);
		bzero(msg, m_size);
		sprintf(msg, "%s 200 Document Follows\r\nContent-Type:%s\r\nContent-Length:%lu\r\nConnection: Closed\r\n\r\n", ver, cont_type, stats.st_size);
		memmove(msg+strlen(msg), buf, stats.st_size+1);
	}
	else
	{
		m_size = strlen(V0MSG) + strlen(ver) + strlen(cont_type) + c_fsize + stats.st_size + 50;
		msg = malloc(m_size);
		bzero(msg, m_size);
		sprintf(msg, "%s 200 Document Follows\r\nContent-Type:%s\r\nContent-Length:%lu\r\n\r\n", ver, cont_type, stats.st_size);
		memmove(msg+strlen(msg), buf, stats.st_size+1);
	}
	//sending the msg
	write(connfd, msg, m_size);
	//freeing variables
	printf("filepath: %s\n", fpath);
	printf("m_size: %d\n",m_size);
	printf("f_size:%lu\n",stats.st_size);
	printf("meth: %s\nuri: %s\n",meth,uri);
	printf("ver: %s\n",ver);
	printf("cont_type: %s\n\n\n",cont_type);
	fclose(fp);
	free(buf);
	free(uri);
	free(msg);
	free(fpath);
	return 0;
}

/*
 * open_listenfd - open and return a listening socket on port
 * Returns -1 in case of failure
 */
int open_listenfd(int port)
{
    int listenfd, optval=1;
    struct sockaddr_in serveraddr;

    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
	printf("failed to create socket descriptor\n");
        return -1;
    }

    /* Forces Bind SO_REUSEADDR is for FTP that requires reuse of same socket*/
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval , sizeof(int)) < 0)
    {
	printf("failed to force re-binding to IP address\n");
        return -1;
    }

    /* listenfd will be an endpoint for all requests to port
       on any IP address for this host */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    if (bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
    {
	    printf("failed to bind\n");
	    return -1;
    }

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0)
    {
	    printf("failed to listen\n");
        return -1;
    }
    return listenfd;
} /* end open_listenfd */

