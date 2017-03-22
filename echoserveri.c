/*
 * echoserveri.c - An iterative echo server
 */

#include "csapp.h"

#define MAX_NAME_LEN 256
#define NPROC 4

void echo(int connfd);
void handle_connection(int listenfd){
    int connfd;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];
    socklen_t clientlen;

    clientlen = (socklen_t)sizeof(clientaddr);

    while (1) {
        printf("Waiting\n");
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        printf("New connection.\n");
        /* determine the name of the client */
        Getnameinfo((SA *) &clientaddr, clientlen,
                    client_hostname, MAX_NAME_LEN, 0, 0, 0);
        
        /* determine the textual representation of the client's IP address */
        Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
                  INET_ADDRSTRLEN);
        
        printf("server connected to %s (%s)\n", client_hostname,
               client_ip_string);

        echo(connfd);
        Close(connfd);
    }
}

/* 
 * Note that this code only works with IPv4 addresses
 * (IPv6 is not supported)
 */
int main(int argc, char **argv)
{
    int listenfd, port;
    
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    pid_t children[NPROC];

    port = atoi(argv[1]);
    listenfd = Open_listenfd(port);

    int i;
    for(i=0; i<NPROC; i++){
        children[i] = Fork();
        if(children[i] == 0){
            handle_connection(listenfd);
        }
    }
        
    

    wait(NULL); /* Wait for all children */
    exit(0);
}

