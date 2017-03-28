/*
 * echo - read and echo text lines until client closes connection
 */
#include "csapp.h"
#include <string.h>

#define INVALID_COMMAND "Invalid command\n"
void echo(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        printf("server received %u bytes\n", (unsigned int)n);
        Rio_writen(connfd, buf, n);
    }
}

int readFile( char* filename, char** content){
	int size = strlen(filename)+1;
	*content = malloc(size);
	sprintf(*content, filename);
	*content[strlen(*content)] = '\0';
	return size;



	// FILE* fichier;

	// fichier = fopen(command, "r");
 //    if(fichier == NULL){
 //    	printf("Erreur d'ouverture de %s\n", command);
 //    }

 //    while (fread(buff, 1, 1, fichier) == 1) {
 //        Rio_writen(connfd, buff, 1);
 //    }
}


void lire(int connfd){
	size_t n;
	int answer_size;
    char command[MAXLINE], *buff, exit = 0, *answer, *content;
    rio_t rio;


    Rio_readinitb(&rio, connfd);
    while(!exit){
	    n = Rio_readlineb(&rio, command, MAXLINE);
	    command[strlen(command)-1] = '\0';
	    if(strcmp(command, "quit") == 0){
	    	exit = 1;
	    }else if (command[0] == 'g' && command[1] == 'e' && command[2] == 't' && command[3] == ' '){
	    	int filename_size = strlen(command) - 4 + 1;
	    	char *filename = malloc(filename_size);
	    	printf("Coucou\n");
	    	strcpy(filename, &command[4]);


		    printf("About to read the file %s\n", filename);
		    answer_size = readFile(filename, &content);
		    printf("Size : %d - content %s\n", answer_size, content);
		    Rio_writen(connfd, content, answer_size);
		    free(content);
		    exit=1;
		}else{
			Rio_writen(connfd, INVALID_COMMAND, strlen(INVALID_COMMAND)+1);
		}
	}
}

