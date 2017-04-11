/*
 * echo - read and echo text lines until client closes connection
 */
#include "csapp.h"
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>

#define MAXCOMMAND 10
#define BLOCK_SIZE 40

char *string_to_send = NULL;

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

/* Avec cette fonction, l'envoi est différé */
void send_to_rio(char* string){
	if(string_to_send == NULL){
		string_to_send = malloc(strlen(string)+1);
		strcpy(string_to_send, string);
	}else{
		string_to_send = realloc(string_to_send, strlen(string_to_send) + strlen(string) + 1);
		strcpy(string_to_send + strlen(string_to_send), string);
	}
}

char last_char_off(char *string){
	if(string == NULL || strlen(string) == 0)
		return '\0';
	return string[strlen(string)-1];
}

/* 0 : n'existe pas, 1 : fichier, 2 : répertoire */
int exist_file(char *path){
	struct stat s;
	int err = stat(path, &s);
	if(-1 == err) {
	    if(ENOENT == errno) {
	        return 0;
	    } else {
	        perror("stat");
	        exit(1);
	    }
	} else {
	    if(S_ISDIR(s.st_mode)) {
	        return 2;
	    } else {
	        return 1;
	    }
	}
}


void set_working_directory(char *working_directory, char *path){
	if(exist_file(path) != 2){
        send_to_rio("Directory not found : ");
        send_to_rio(path);
        send_to_rio("\n");
        return;
	}

	working_directory = realloc(working_directory, strlen(path) + 1);
	strcpy(working_directory, path);
}

void init_working_directory(char *working_directory){
	int size = 1000;
	char cwd[size];
	getcwd(cwd, size);
	set_working_directory(working_directory, cwd);
}

void change_directory(char *working_directory, char *path){
	if(path[strlen(path)-1] == '/') /* slash final */
		path[strlen(path)-1] = '\0';
	

	if(path[0] == '/'){
		set_working_directory(working_directory, path);
	} else if(strcmp(path, "..") == 0){
		while(working_directory[strlen(working_directory)-1] != '/'){
			working_directory[strlen(working_directory)-1] = '\0';
		}
		if(strcmp(working_directory, "/") != 0){
			working_directory[strlen(working_directory)-1] = '\0';
		}
		working_directory = realloc(working_directory, strlen(working_directory) + 1);
	} else {
		char new_working_directory[strlen(working_directory) + strlen(path) + 2];
		strcpy(new_working_directory, working_directory);
		strcpy(new_working_directory + strlen(new_working_directory), "/");
		strcpy(new_working_directory + strlen(new_working_directory) - 1 + 1, path);
		set_working_directory(working_directory, new_working_directory);
	}
}

void list_directory(char *path){
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir (path)) != NULL) {
	  while ((ent = readdir (dir)) != NULL) {
	    send_to_rio(ent->d_name);
	    send_to_rio("\n");
	  }
	  closedir (dir);
	} else {
	  perror ("Directory not found. This error is not supposed to appear");
	  exit(0);
	}
}

void remove_file(char *working_directory, char *filename, int recursivity){
	char path[strlen(working_directory) + strlen(filename) + 1 + 1];
	strcpy(path, working_directory);
	strcpy(path + strlen(working_directory), "/");
	strcpy(path + strlen(working_directory) + 1, filename);

	if(exist_file(path) == 0){
		send_to_rio("File not found : ");
		send_to_rio(path);
		send_to_rio("\n");
	} else if (exist_file(path) == 2 && recursivity == 0){
		send_to_rio("To delete a directory use 'rm -r'\n");
	}else{
		remove(path);
	}
}

void liberer(char **table){
	int i = 0;
	while(table[i] != NULL){
		free(table[i]);
		i++;
	}
}



void send_prompt(char* working_directory, char last_char_sent){
	if(last_char_sent != '\n')
		send_to_rio("\n");

	send_to_rio("ftp - ");
	send_to_rio(working_directory);
	send_to_rio(" > ");
}

void flush(char *working_directory, int connfd){

	send_prompt(working_directory, last_char_off(string_to_send));

	Rio_writen(connfd, string_to_send, strlen(string_to_send)+1);
	string_to_send = NULL;
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
	char tmp[BLOCK_SIZE];
	file = fopen(filename, "r");
	int continuer = 1;

	while(continuer){
		nb_read = fread(tmp, BLOCK_SIZE, 1, file);
		if(feof(file)){
			send_to_rio(tmp);
			continuer = 0;
		}else{
			Rio_writen(connfd, tmp, BLOCK_SIZE);
		}
	}

}

void welcome(char *working_directory, int connfd){
	send_to_rio("Connecté au server FTP\n");
	flush(working_directory, connfd);
}


void lire(int connfd){
	size_t n;
    char buf[MAXLINE];
    char *commands[MAXCOMMAND];
    rio_t rio;

    char *working_directory = malloc(0); /* Pour que realloc fonctionne */
    init_working_directory(working_directory);

    Rio_readinitb(&rio, connfd);
    welcome(working_directory, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
	    buf[strlen(buf)-1] = '\0';
        printf("server received %u bytes\n", (unsigned int)n);

        split(buf, commands);

        if(strcmp(commands[0], "quit") == 0 || strcmp(commands[0], "bye") == 0 || strcmp(commands[0], "exit") == 0){
			send_to_rio("Exit\n");
        	break;
        }else if(strcmp(commands[0], "get") == 0){
        	if(commands[1] == NULL){
        		send_to_rio("Need file as argument\n");
        	}else{
        		char filename[strlen(working_directory) + strlen(commands[1]) + 1 + 1];
        		strcpy(filename, working_directory);
        		strcpy(filename + strlen(working_directory), "/");
        		strcpy(filename + strlen(working_directory) + 1, commands[1]);
        		if(exist_file(filename) != 1){
        			send_to_rio("File not foud : ");
        			send_to_rio(filename);
        			send_to_rio("\n");
        		}else{
        			send_file(connfd, filename);
        		}
        	}

        }else if(strcmp(commands[0], "cd") == 0){
        	if(commands[1] == NULL){
        		init_working_directory(working_directory);
        	}else{
        		change_directory(working_directory, commands[1]);
        	}
        }else if(strcmp(commands[0], "pwd") == 0){
	        send_to_rio(working_directory);
        }else if(strcmp(commands[0], "ls") == 0){
        	if(exist_file(working_directory) != 2){
		        send_to_rio("Directory not found : ");
		        send_to_rio(working_directory);
		        send_to_rio("\n");
        	} else {
        		list_directory(working_directory);
        	}
        } else if(strcmp(commands[0], "mkdir") == 0){
        	if(commands[1] == NULL){
        		send_to_rio("Need name as argument\n");
        	}else{
	        	char filename[strlen(working_directory) + strlen(commands[1]) + 1 + 1];
	    		strcpy(filename, working_directory);
	    		strcpy(filename + strlen(working_directory), "/");
	    		strcpy(filename + strlen(working_directory) + 1, commands[1]);
	    		if(exist_file(filename) != 0){
	    			send_to_rio("File exitsts : ");
	    			send_to_rio(filename);
	    			send_to_rio("\n");
	    		}else{
	    			mkdir(filename, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	        	}
	        }
        } else if(strcmp(commands[0], "rm") == 0){
        	if(commands[1] == NULL){
        		send_to_rio("Need name as argument\n");
        	}else if(strcmp(commands[1], "-r") == 0){
        		if(commands[2] == NULL){
        			send_to_rio("Need name as argument\n");
        		} else {
        			remove_file(working_directory, commands[2], 1);
        		}
        	} else {
        		remove_file(working_directory, commands[1], 0);
        	}
        }
        else{
        	send_to_rio("Invalid command\n");
        }

        flush(working_directory, connfd);
        liberer(commands);
    }
    free(working_directory);
}