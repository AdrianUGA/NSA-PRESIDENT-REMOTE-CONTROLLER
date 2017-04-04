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
	Rio_writen(connfd, string, strlen(string)+1);
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

int readFile( char* filename, char* content){
	strcpy(content, "Coucou");
	return 2;
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
        	readFile(commands[1], file_content);
        	printf("%s\n", file_content);
    	    send_to_rio(connfd, "file_content");

        }else{
        	send_to_rio(connfd, "");
        }
        liberer(commands);
    }
}

void lire2(int connfd){
	size_t n;
	int answer_size;
	char action[5];
    char command[MAXLINE], *buff, leave = 0, *answer, *content;
    rio_t rio;


    Rio_readinitb(&rio, connfd);
    while(!leave){

	    n = Rio_readlineb(&rio, command, MAXLINE);
	    command[strlen(command)-1] = '\0';
	    memcpy(action, &buff[0], 4);
		action[4] = '\0';

	    if(strcmp(command, "quit") == 0){
	    	leave = 1;
	    }else if (strcmp(action, "get ") == 0){
	    	printf("Coucou\n");
	    	int filename_size = strlen(command) - 4 + 1;
	    	char *filename = malloc(filename_size);
	    	strcpy(filename, &command[4]);


		    printf("About to read the file %s\n", filename);
		    answer_size = readFile(filename, content);
		    printf("Size : %d - content %s\n", answer_size, content);
		    Rio_writen(connfd, content, answer_size);
		    free(content);
		    leave=1;
		}else{
			Rio_writen(connfd, INVALID_COMMAND, strlen(INVALID_COMMAND)+1);
		}
	}
}

