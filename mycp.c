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

#define BUF_SIZE 1024
char temp_buf[BUF_SIZE];
int fd_1, fd_2;
int* fd1 = &fd_1;
int* fd2 = &fd_2;

void def_file_copy(int* fd1, int* fd2, char* temp_buf) {
	ssize_t str, stw;
	ssize_t total_written = 0; 
	while ((str = read(*fd1, temp_buf, BUF_SIZE)) > 0) {
		while (total_written < str) {
			stw = write(*fd2, temp_buf + total_written, str - total_written);
			if (stw == -1) {
				if (errno == EINTR) {
					continue;
				}
				fprintf(stderr, "Ошибка записи: %s, CODE %d\n", strerror(errno), errno);
				close(*fd1);
				close(*fd2);
				exit(EXIT_FAILURE);
			}
			total_written += stw;
		}
	}
}

char path_base[PATH_MAX];
char path_two[PATH_MAX];
size_t path_base_index = 0;
size_t path_two_index = 0;

DIR* dir; 
DIR* temp_dir;

DIR* folder;

DIR* faf_copy(DIR* d, char* a, char* b, size_t a_size, size_t b_size) {
		struct stat sb = {0};
		struct dirent* dr = {0};
		char temp_a[PATH_MAX];
		char temp_b[PATH_MAX];
		snprintf(temp_a, sizeof(temp_a), "%s", a);
		snprintf(temp_b, sizeof(temp_b), "%s", b);		
		folder = NULL;		

		while ((dr = readdir(d)) != NULL) {
			if (strcmp(dr->d_name, ".") == 0 || strcmp(dr->d_name, "..") == 0) {
				continue;
			}
			//fprintf(stderr, "DEBUG before snprintf (while): path = %s, d_name = %s\n", a, dr->d_name);
			snprintf(a, a_size, "%s/%s", temp_a, dr->d_name); 
			//fprintf(stderr, "DEBUG after snprintf  (while): path = %s, d_name = %s\n", a, dr->d_name);
			if (lstat(a, &sb) == -1) {
				perror("lstat error");
				exit(EXIT_FAILURE); 
			}
			if (S_ISDIR(sb.st_mode)) {
				folder = opendir(a);
				snprintf(b + path_two_index, b_size, "%s/%s", temp_b, dr->d_name);
				path_two_index = sizeof(b);
				snprintf(a + path_base_index, b_size, "%s/%s", temp_a, dr->d_name);
				path_base_index = sizeof(a);
				mkdir(b, 0755);
				continue;
			}
			char temp_for_fd[PATH_MAX];
			snprintf(temp_for_fd, sizeof(temp_for_fd), "%s/%s", temp_a, dr->d_name);
			fd_1 = open(temp_for_fd, O_RDONLY);
			if (fd_1 == -1) {
				perror("fd 1 error");
				close(fd_1);
				exit(EXIT_FAILURE);
			}

			snprintf(temp_for_fd, sizeof(temp_for_fd), "%s/%s", temp_b, dr->d_name);
			fd_2 = open(temp_for_fd, O_WRONLY | O_TRUNC | O_CREAT, 0644);
			if (fd_2 == -1) {
				perror("fd 2 error");
				close(fd_2);
				exit(EXIT_FAILURE);
			}
			def_file_copy(fd1, fd2, temp_buf);
		}
			fprintf(stderr, "PATH BASE BEFORE RETURN = %s", path_base);
			return folder;
} 




int main(int argc, char *argv[]) {
	int opt;
	bool recursive_flag = false;
	while ((opt = getopt(argc, argv, "rR")) != -1) {
		switch(opt) {
			case 'r':
			case 'R':
			recursive_flag = 1;
			break;
		default:
			fprintf(stderr, "Использование %s -r <источник> <цель>\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	} 
	if (argc - optind < 2) {
		fprintf(stderr, "Ошибка: Не указан источник или цель\n");
		exit(EXIT_FAILURE);
	}
	char *rf = argv[optind];
	char *wf = argv[optind + 1];

	if (recursive_flag) {
		struct dirent *rd = {0};
		struct dirent *temp_rd = {0};
		size_t base_size = sizeof(path_base);
		size_t two_size = sizeof(path_two);

		snprintf(path_base, sizeof(path_base), "%s", rf);
		snprintf(path_two, sizeof(path_two), "%s", wf);
		
		dir = opendir(path_base);		
		temp_dir = dir;

		while ((rd = readdir(dir)) != NULL) {
			if (temp_dir == NULL) {
			return 1;
			} else {
			fprintf(stderr, "DEBUG faf_copy: path_base = %s\n", path_base);
			temp_dir = faf_copy(temp_dir, path_base, path_two, base_size, two_size);
			if (temp_dir != NULL) {
				int tdfd = dirfd(temp_dir);
				if (tdfd == -1) {
					perror("temp_dir dirfd error");
					exit(EXIT_FAILURE);
				}
				char proc_path[64];
				snprintf(proc_path, sizeof(proc_path), "/proc/self/fd/%d", tdfd);
				char act_path[PATH_MAX];
				ssize_t len = readlink(proc_path, act_path, sizeof(act_path) - 1);
				if (len != -1) {
					act_path[len] = '\0';
					fprintf(stderr, "debug full path %s\n", act_path); 
				}
				char* last_slash = strrchr(act_path, '/');
				char *dir_name;
				if (last_slash != NULL ) {
					dir_name = last_slash + 1;
				} else {
					dir_name = act_path;
				}
				fprintf(stderr, "PATH_BASE before + dir_name = %s\n", path_base);
				snprintf(path_base, base_size, "%s/%s", path_base, dir_name);
				fprintf(stderr, "PATH_BASE IN IF TEMP_DIR = %s\n", path_base);
			}
			}  
		} 
	}		
	
	close(*fd1);
	close(*fd2);
}












