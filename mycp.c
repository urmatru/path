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
char buf_for_fd2[PATH_MAX];
char path[PATH_MAX];
int fd_1, fd_2;
int* fd1 = &fd_1;
int* fd2 = &fd_2;
char* wf;
DIR* dir;
DIR* dir2;
size_t pindex = 0;
bool is_folder = false;

char buf[PATH_MAX];

void def_files_copy(struct dirent* rd, DIR* dir, DIR* dir2, char* temp_buf) {
	fprintf(stderr, "===================11====\n");
	while ((rd = readdir(dir)) != NULL) {
		if (rd->d_type != DT_DIR) {
			int dir_fd = dirfd(dir);
			fd_1 = openat(dir_fd, rd->d_name, O_RDONLY);
			if (fd_1 == - 1) {
				perror("fd_1 error");
			}
			int dir_fd2 = dirfd(dir2);
			fd_2 = openat(dir_fd2, rd->d_name, O_WRONLY | O_TRUNC | O_CREAT, 0644);
			if (fd_2 == - 1) {
				perror("fd_2 error");
			}

// T		
			ssize_t len = 0;

			char linkpath[64];
			snprintf(linkpath, sizeof(linkpath), "/proc/self/fd/%d", fd_1);
			//fprintf(stderr, "FD_1 PATH BEFORE READLINK: %s\n", linkpath);
			len = readlink(linkpath, buf, sizeof(buf) - 1);
			//fprintf(stderr, "FD_1 PATH BEFORE READLINK IN BUF: %s\n", buf);
			if (len != -1) {
				buf[len] = '\0';
				fprintf(stderr, "PATH for fd_1: %s\n", buf);
			} else {
				fprintf(stderr, "path getting error for fd_1");
			}

			snprintf(linkpath, sizeof(linkpath), "/proc/self/fd/%d", fd_2);
			fprintf(stderr, "FD_2 PATH BEFORE READLINK: %s\n", linkpath);
			len = readlink(linkpath, buf, sizeof(buf) - 1);
			if (len != -1) {
				buf[len] = '\0';
				fprintf(stderr, "PATH for fd_2:%s\n", buf);
			} else {
				fprintf(stderr, "path getting error for fd_2");
			}

////			 
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
		} else {
			
			is_folder = true;
		}		
	} 	
}

char temp_path[PATH_MAX] = {0};

void folder_copy(char* path) {
	fprintf(stderr, "PATH AT START OF FOLDER COPY FUNC: %s\n", path);
	struct dirent* rd = {0};
	dir = opendir(path);

	char* first_slash = strchr(path, '/');
	if (first_slash != NULL) {
		snprintf(buf_for_fd2, sizeof(buf_for_fd2), "/%s", first_slash);  
		fprintf(stderr, "CHECK WHAT IS BUF_FOR_FD2 %s\n", buf_for_fd2);
		if (mkdir(buf_for_fd2, 0777) == -1 ) {
			if (errno != EEXIST) {
			perror("CAN'T CREATE FOLDER\n");
			}
		} else {
			fprintf(stderr, "FOLDER %s CREATED!\n", buf_for_fd2);
		}
	} else {
		if (mkdir(buf_for_fd2, 0777) == -1 ) {
			if (errno != EEXIST) {
			perror("CAN'T CREATE FOLDER\n");
			}
		} else {
			fprintf(stderr, "FOLDER %s CREATED!\n", buf_for_fd2);
		}		
	}


	dir2 = opendir(buf_for_fd2);
	struct dirent *rd_temp = rd;
	is_folder = false;
	def_files_copy(rd_temp, dir, dir2, temp_buf);

	
	if (!is_folder) {
		fprintf(stderr, "PATH AT END OF FOLDER COPY FUNC: %s", path);
		exit(EXIT_SUCCESS);
	} else {
		while (1) { 
		if ((rd = readdir(dir)) == NULL) {
			perror("WHATS WRONG WITH RD???");
			exit(EXIT_FAILURE);
		} else {
			fprintf(stderr, "COUNT!!\n");			
			if (rd->d_type == DT_DIR) {
				fprintf(stderr, "IF WHILE !!!!!!!!!!!! CHECK\n");
				snprintf(temp_path, sizeof(temp_path), "/%s", rd->d_name);
				folder_copy(temp_path);
			}
			strcpy(temp_path, path);
		} 
		} 	
	}
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
	wf = argv[optind + 1];

	if (recursive_flag) {
		snprintf(path, sizeof(path), rf);
		strncat(temp_path, path, sizeof(path));
		snprintf(buf_for_fd2, sizeof(buf_for_fd2), wf);
		folder_copy(path);
	}

	fprintf(stderr, "CHECK ++++++++++++++");

	close(*fd1);
	close(*fd2);
}












