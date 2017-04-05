/*
 * echo - read and echo text lines until client closes connection
 */
#include "csapp.h"
#include <string.h>

#define INVALID_COMMAND "Invalid command\n"
#define MAXCOMMAND 10


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
	Rio_writen(connfd, string, strlen(string)); /* On veille à ne pas envoyer le zéro de fin de cha^ine */
	Rio_writen(connfd, "\n", 1);
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

char* readFile(char* filename){
	char *content = malloc(strlen(filename)+1);
	strcpy(content, filename);
	return content;
}


void lire(int connfd){
	size_t n;
    char buf[MAXLINE], *file_content;
    char *commands[MAXCOMMAND];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
	    buf[strlen(buf)-1] = '\0';
        printf("server received %u bytes\n", (unsigned int)n);

        split(buf, commands);

        if(strcmp(commands[0], "quit") == 0){
			send_to_rio(connfd, "Exited");
        	break;
        }else if(strcmp(commands[0], "get") == 0){
        	if(commands[1] == NULL){
        		send_to_rio(connfd, "Need file as argument");
        		continue;
        	}
        	file_content = readFile(commands[1]);
        	printf("%s\n", file_content);
    	    send_to_rio(connfd, "file_content");

        }else{
        	send_to_rio(connfd, "");
        }
        liberer(commands);
    }
}