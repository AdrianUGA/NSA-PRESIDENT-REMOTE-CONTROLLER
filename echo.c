/*
 * echo - read and echo text lines until client closes connection
 */
#include "csapp.h"
#include <string.h>

#define MAXCOMMAND 10
#define BLOCK_SIZE 40


void split(char *string, char **table){
	char tmp[MAXLINE];
	int stringIndex=0, wordNumber=0, curentWordIndex=0, leave=0;
	while(!leave){
		if(string[stringIndex] == ' ' || string[stringIndex] == 0){
			tmp[curentWordIndex] = '\0'; curentWordIndex++;
			table[wordNumber] = malloc(curentWordIndex * sizeof(char));
			strcpy(table[wordNumber], tmp);
			curentWordIndex = 0;
			wordNumber++;
			if(string[stringIndex] == 0){
				leave=1;
				table[wordNumber] = NULL;
			}
		}else{
			tmp[curentWordIndex] = string[stringIndex];
			curentWordIndex++;
		}
		stringIndex++;
	}
}

void liberer(char **table){
	int i = 0;
	while(table[i] != NULL){
		free(table[i]);
		i++;
	}
}

void send_to_rio(int connfd, char* string){
	Rio_writen(connfd, string, strlen(string)+1); /* On veille à bien envoyer le zéro de fin de cha^ine */
}

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

char* send_file(int connfd, char* filename){

	int nb_read;
	FILE *file;
	char tmp[BLOCK_SIZE+1];
	file = fopen(filename, "r");

	do{
		nb_read = fread(tmp, BLOCK_SIZE, 1, file);
		tmp[BLOCK_SIZE] = '\0';
		send_to_rio(connfd, tmp);
	}while(nb_read == BLOCK_SIZE);


}


void lire(int connfd){
	size_t n;
    char buf[MAXLINE];
    char *commands[MAXCOMMAND];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
	    buf[strlen(buf)-1] = '\0';
        printf("server received %u bytes\n", (unsigned int)n);

        split(buf, commands);

        if(strcmp(commands[0], "quit") == 0 || strcmp(commands[0], "bye") == 0 || strcmp(commands[0], "exit") == 0){
			send_to_rio(connfd, "Exit\n");
        	break;
        }else if(strcmp(commands[0], "get") == 0){
        	if(commands[1] == NULL){
        		send_to_rio(connfd, "Need file as argument\n");
        		continue;
        	}
        	send_file(connfd, commands[1]);

        }else{
        	send_to_rio(connfd, "Invalid command\n");
        }
        liberer(commands);
    }
}