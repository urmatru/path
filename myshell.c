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
bool redirect = false;

void line_parser(char* line, char* sh_argv[]) {
	int i = 0;
	if (*line == '"') {
		char* line_copy = line;
		while (*line_copy != '\"') { // ОТСЮДА
			line_copy++;
		}
		sh_argv[i] = strtok(line, line_copy + 1);
	} else {
		sh_argv[i] = strtok(line, " ");
	}

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


typedef void (*builtinc_func)(char** argv);

void builtin_cd (char** argv) {
	if (argv[1] == NULL) {
		fprintf(stderr, "cd: missing argument\n");
		return;
	}
	if (chdir(argv[1]) == -1) {
		perror("cd");
		return;
	}
}

/* void builtin_ls (char** argv) {
	if (getdents64() == -1) {
		perror("ls");
		return;
	}
} */

void builtin_exit(char** argv) {
	exit(0);
}

void remove_quotes(char *arg) {
	if (!arg) return;
	
	size_t rind = 0;
	size_t wind = 0;
	bool in_single = false;
	bool in_double = false;

	while (arg[rind] != '\0') {
		char c = arg[rind];
		if (c == '\'' && !in_double) {
			in_single = !in_single;
		} else if (c == '"' && !in_single) {
			in_double = !in_double;
		} else {
			arg[wind] = c;
			wind++;
		}
		rind++;
	}
	
	arg[wind] = '\0';
}

void clean_argv_quotes(char **argv) {
	if (!argv) return;

	for (int i = 0; argv[i] != NULL; i++) {
		remove_quotes(argv[i]);
	}
}

void builtin_echo(char** argv) {
	int i = 1;
	int fd = 0;	
	char** argv_copy;

	if (redirect) {
		argv_copy = argv;
		while (*argv_copy) {
		argv_copy++;
		}

		fprintf(stderr, "FILE ARGV COPY: %s\n", *--argv_copy); 
		fd = open(*argv_copy, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd == -1) {
			perror("FD OPEN ERROR\n");
			exit(EXIT_FAILURE);
		}
		dup2(fd, STDOUT_FILENO);
	}

	argv_copy = argv;
	clean_argv_quotes(argv_copy);

	while (argv_copy[i] != NULL) {
		fprintf(stderr, "argv[i]: %s\n", argv[i]);
		fputs(argv[i], stdout);
		if (argv[i + 1] != NULL) {
			putchar(' ');
		}
		i++;
		argv_copy++;
	}
	exit(EXIT_SUCCESS);
	putchar('\n');
}

struct builtin {
	char *name;
	builtinc_func func;
};

int main(/*int argc, char* argv[]*/) {
	printf("================================SHELL================================\n");
	while (1) {
		fputs(" & ", stdout);

		size_t n = 0;
		char *line = NULL;
		getline(&line, &n, stdin);
		line[strcspn(line, "\n")] = 0;

		/*if (strcmp(line, "exit") == 0) {
			exit(0);
		} */
	
		struct builtin builtins[] = {
			{"cd", builtin_cd},
			//{"ls", builtin_ls},
			{"echo", builtin_echo},
			{"exit", builtin_exit}, 
			{NULL, NULL}
		};
		char* sh_argv[10] = {0};
		line_parser(line, sh_argv);

		char** current_arg = sh_argv;
		char* filename;

		while (*current_arg != NULL) {
			if (strcmp(*current_arg, ">") == 0) {
			filename = *(current_arg + 1);
			*current_arg = strdup(filename);
			*(current_arg + 1) = NULL ;
			redirect = true;
			break;
			} else if (strcmp(*current_arg, "<") == 0) {
			filename = *(current_arg + 1);
			break;	
			}
			current_arg++;
		}		

		int i = 0;
		bool struct_dect = false;
		while (builtins[i].name != NULL) {
			if (strcmp(sh_argv[0], builtins[i].name) == 0) {
				builtins[i].func(sh_argv);
				struct_dect = true;
				break;	
			}
			i++;
		}

		if (!struct_dect) {

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
			
}
