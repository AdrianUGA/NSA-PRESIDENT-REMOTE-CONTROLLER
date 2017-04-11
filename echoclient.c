/*
 * echoclient.c - An echo client
 */
#include "csapp.h"
#define PORT 2121

int main(int argc, char **argv)
{
    int clientfd, port, leave=0;
    char *host, buf[MAXLINE];
    rio_t rio;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = PORT;

    /*
     * Note that the 'host' can be a name or an IP address.
     * If necessary, Open_clientfd will perform the name resolution
     * to obtain the IP address.
     */
    clientfd = Open_clientfd(host, port);
    
    /*
     * At this stage, the connection is established between the client
     * and the server OS ... but it is possible that the server application
     * has not yet called "Accept" for this connection
     */
    printf("client connected to server OS\n"); 
    
    Rio_readinitb(&rio, clientfd);
    
    while (!leave) {
        if (read(clientfd, buf, MAXLINE) > 0) {
            if(strcmp(buf, "\n") != 0)
                Fputs(buf, stdout);
        } else { /* the server has prematurely closed the connection */
            break;
        }

        
        Fgets(buf, MAXLINE, stdin);

        if(strcmp(buf, "quit\n") == 0 || strcmp(buf, "bye\n") == 0 || strcmp(buf, "exit\n") == 0){
            leave = 1;
        }

        Rio_writen(clientfd, buf, strlen(buf));

        
    }
    Close(clientfd);
    exit(0);
}