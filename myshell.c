#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h> 
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>

extern char** environ;	

void line_parser(char* line, char* sh_argv[]) {
	int i = 0;
	sh_argv[i] = strtok(line, " ");

	while (sh_argv[i] != NULL) {
		i++;
		sh_argv[i] = strtok(NULL, " ");
	}		

}

int main(int argc, char* argv[]) {
	printf("================================SHELL================================\n");
	while (1) {
		fputs(" & ", stdout);
		
		(void)argc;
		size_t n = 0;
		char *line = NULL;
		getline(&line, &n, stdin);
		line[strcspn(line, "\n")] = 0;
	
		char* sh_argv[10] = {0};
		line_parser(line, sh_argv);		
		
		pid_t pid = fork();
		if (pid < 0) {
			perror("fork failed");
		} else if (pid == 0) {
			char* path_s = getenv("PATH");
				if (path_s != NULL && strchr(sh_argv[0], '/') == NULL) {
				while (1) {
				char* path_s_copy = strdup(path_s);
				char* dir = strtok(path_s_copy, ':');
				fprintf(stderr, "=PATH: %s", dir);
				if (dir == NULL) {
					fprintf(stderr, "PATH END\n");
					break;
				}
				char* last_slash = strrchr(dir, '/');
				strcat(last_slash, sh_argv[0]);
				}
				}
			}
			exit(EXIT_SUCCESS);

		
			execve(sh_argv[0], sh_argv, environ);
			perror("execve failed");
			exit(EXIT_FAILURE);
		} 
		else {
			int status;
			waitpid(pid, &status, 0);
		}
	} 
			
}
