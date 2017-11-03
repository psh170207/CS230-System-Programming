/*
 * proxy.c - implement the proxy server routine.
 * 
 * Writer : cs20150326 Park Si Hwan
 *
 */
#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

void doit(int fd);
void parse_req(char *uri, char *port, char *hostname, char *request);
int parse_uri(char *uri, char *filename, char *cgiargs);

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

int main(int argc, char **argv)
{
    typedef struct sockaddr SA;
	int listenfd,connfd;
	char hostname[MAXLINE],port[MAXLINE];
	struct sockaddr_storage clientaddr;
	socklen_t clientlen;
	pid_t pid; // pid for fork a new child process
	
	/* If argc != 2, print error message and exit */
	if(argc != 2){
		fprintf(stderr,"usage: %s <port>\n",argv[0]);
		exit(1);
	}
	
	listenfd = Open_listenfd(argv[1]);
	while(1){
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
		
		/* fork a new child for concurrency */
		if((pid = fork())==0){
			Close(listenfd);
			Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,0);
			printf("Accepted connection from (%s, %s)\n",hostname, port);
			doit(connfd);
			Close(connfd);
			exit(0);
		}
		Close(connfd); // close connfd to prevent memory leak
	}
}

/*
 * doit - doit is main routine for proxy's role.
 *        read client's request, parsing them and connect with end server, send the parsed
 *        request to end server, read the end server's response, finally send the end server's
 *        respones to client.
 */
void doit(int fd)
{
	char buf[MAXLINE], method[MAXLINE], version[MAXLINE], uri[MAXLINE];
	char port[MAXLINE], hostname[MAXLINE], request[MAXLINE], fwreq[MAXLINE];
	rio_t rio;
	int clientfd,num;
	
	/* Read the request from client */
	Rio_readinitb(&rio, fd);
	if(!Rio_readlineb(&rio, buf, MAXLINE)) return;
	printf("%s", buf);

	/* Parse the HTTP request */
	sscanf(buf,"%s %s %s", method, uri, version);
	parse_req(uri,port,hostname,request);
	
	/* Connect to end server */
	clientfd = Open_clientfd(hostname,port);
	
	/* Initialize the rio connect to clientfd and fwreq(fowarding request) */
	Rio_readinitb(&rio, clientfd);
	memset(fwreq,0,MAXLINE);

	/* Make HTTP request header */
	strcat(fwreq, method);
	strcat(fwreq, " ");
	strcat(fwreq, request);
	strcat(fwreq, " ");
	strcat(fwreq, "HTTP/1.0\r\n");
	sprintf(fwreq, "%sHost: %s\r\n",fwreq,hostname);
	strcat(fwreq, user_agent_hdr);
	strcat(fwreq, "Connection: close\r\n");
	strcat(fwreq, "Proxy-Connection: close\r\n");
	strcat(fwreq, "\r\n");
	
	/* Send the HTTP request to end server */
	Rio_writen(clientfd, fwreq, (int)strlen(fwreq)+1);

	/* Read end server's response and send to client */
	while((num = Rio_readlineb(&rio, buf, MAXLINE))>0){
		Rio_writen(fd,buf,num);
		Fputs(buf, stdout);
	}
	Close(clientfd);
}

/*
 * parse_req - parsing the HTTP request from client to make new HTTP request for end server
 *             and find hostname and port for connect with end server
 */
void parse_req(char *uri, char *port, char *hostname, char *request)
{

	int i,j,si; // string iterators
	int cnt1 = 0; // count the number of occurrences of '/'
	int cnt2 = 0; // count the number of occurrences of ':'
	
	/* Initialize the port, hostname and request string */
	memset(port,0,MAXLINE);
	memset(hostname,0,MAXLINE);
	memset(request,0,MAXLINE);

	for(i=0;i<strlen(uri);i++){
		/* find '/' */
		if(uri[i]=='/'){
			cnt1++;
			/* cnt1 == 2 means next few characters are hostname */ 
			if(cnt1==2){
				j = 0;
				si = i+1;
				while(uri[si]!=':'){
					hostname[j++] = uri[si++];
				}
			}
			/* cnt1 == 3 means next few characters are request file name */
			else if(cnt1 == 3){
				j=0;
				si = i;
				while(si<strlen(uri)){
					request[j] = uri[si];
					j++;
					si++;
				}
			}
		}
		/* find ':' */
		if(uri[i]==':'){
			cnt2++;
			/* cnt2 == 2 means next few characters are port number */
			if(cnt2==2){
				j = 0;
				si = i+1;
				while(uri[si]!='/'){
					port[j++] = uri[si++];
				}
			}
		}

	}

	/* If there's no port, set port to 80 */
	if(!(*port)){
		strcpy(port,"80");
	}
	
	return;
}
