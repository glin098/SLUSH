#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#define LIMIT 256

int child_pid = -1;
int pipefd[2];
FILE* readSide;
FILE* writeSide;

void signalHandler (int sig){
	if(sig == SIGINT){
		printf(" terminate process\n");
	}
}

void createPipe(){
	pipefd[2];
	pipe(pipefd);

	if(pipe(pipefd) < 0){
		perror("pipe not created");
		exit(-1);
	}	
	
	readSide = fdopen(pipefd[0], "r");
	writeSide = fdopen(pipefd[1], "w");

}


void pipeManager(){
	int ret;

	if(child_pid == 0){
		fclose(readSide);
		close(pipefd[0]);

		ret = dup2(pipefd[1], STDOUT_FILENO);

		if (ret == -1){
			perror("could not dup2 write pipe");
			exit(-1);
		}
	}
	else{
		fclose(writeSide);
		close(pipefd[1]);
		
		ret = dup2(pipefd[0], STDIN_FILENO);
		
		if (ret == -1){
			perror("could not dup2 read pipe");
			exit(-1);

		}
	}

}

void pipeCloser(){
	fclose(readSide);
	close(pipefd[0]);

	fclose(writeSide);
	close(pipefd[1]);
}

char* parse(char *cmd, char *delim){

        char *tok = strtok(cmd, delim);
        char *tmp = malloc(LIMIT);

        while(tok != NULL && child_pid == 0) { // if this function is a parent

                strcpy(tmp, tok);

                tok = strtok(NULL, delim);
                if(tok != NULL){
                        createPipe();
                        child_pid = fork();
                        pipeManager();
                }
        }
        return tmp;
}

void execute(char* cmd){
	char *argv[LIMIT];
	int argc = 0;
	char cwd[256];
	const char* delim = " "; 
	
	char* tok = strtok(cmd, delim);

	while(tok != NULL) { // end of argv, null char
		argv[argc] = malloc(strlen(tok) + 1);
		strcpy(argv[argc], tok);
		argc++;
		tok = strtok(NULL, delim);
                
        }

	argv[argc] = NULL;

	int ret = execvp(argv[0], argv);
	if (ret == -1){
		perror("Execution failed");
		exit(-1);

	}
}

int main(int argc, char *argv[]){
	
	signal(2, signalHandler); // CTRL C

	char cmd[LIMIT];
	char* delim = "(";
        char currentDir[LIMIT];	

	while(1){

		printf("slush>> ");

                if(fgets(cmd,LIMIT,stdin) == NULL){
			break;
		}

		char *ptr = strchr(cmd, '\n');
        	
		if (ptr){
                	*ptr  = '\0';
       		}

		char temp[256];
		
		strncpy(temp, cmd, 3);
		temp[3] = '\0';
		
		if(strcmp(temp, "cd ") == 0){
			strcpy(temp, &cmd[3]);
			chdir(temp);
			printf("%s\n", getcwd(currentDir, LIMIT));
		}
		else{
			child_pid = fork();
			waitpid(child_pid, NULL, 0);
		}
		
		if(child_pid == 0){
			char* cmdtok = parse(cmd, delim);
                	
			waitpid(child_pid, NULL, 0);
                	execute(cmdtok);
			
			pipeCloser();
			
			return 0;
		}
	}
	
	return 0;

}
