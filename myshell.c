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

int check_file_rights(const char *filepath) {
	if (access(filepath, F_OK) != 0) {
		printf("ERROR: FILE %s DOESN'T EXISTS\n", filepath);
		return 0; 
	}
	if (access(filepath, X_OK) != 0) {
		printf("ERROR: FILES %s EXISTS, BUT YOU HAVE NO RIGHTS\n", filepath);
		return 0;
	}
	return 1;
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

		if (strcmp(line, "exit") == 0) {
			exit(0);
		}

		char* sh_argv[10] = {0};
		line_parser(line, sh_argv);		
		
		pid_t pid = fork();
		if (pid < 0) {
			perror("fork failed");
		} else if (pid == 0) {
			char* path_s = getenv("PATH");
			char* path_s_copy = strdup(path_s);
			char* dir = strtok(path_s_copy, ":");
			
			if (path_s != NULL && strchr(sh_argv[0], '/') == NULL) {	// execlp(), execvp()
				while (1) {
				dir = strtok(NULL, ":");				
				if (dir == NULL) {
					fprintf(stderr, "PATH END\n");
					break;
				}
				char* end = strrchr(dir, '\0');
				snprintf(end, sizeof(end), "/%s", sh_argv[0]);
				
				if (check_file_rights(dir) == 1) {
					sh_argv[0] = strdup(dir); 
					break;
				}
				}
			}								//
		
			

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
