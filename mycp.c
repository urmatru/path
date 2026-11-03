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
char path[PATH_MAX];
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

int c = 0, a = 0;
DIR* dir;
size_t index = 0;

folder_copy(char* path) {
	struct dirent* rd = {0};
	dir = opendir(path);

	while (rd = opendir(dir) != NULL) {
		struct dirent rd_temp = *rd;
		while (rd_temp = opendir(dir) != NULL && !c ) {
			if (rd_temp->d_type != DT_DIR) {
				fd_1 = openat(rd_temp->d_name, O_RDONLY);
				if (fd == - 1) {
					perror("fd_1 error");
				}
				char* first_slash = strchr(path, '/');
				char buf_for_fd2[PATH_MAX];
				if (first_slash != NULL) {
					char buf_for_fd2[PATH_MAX];
					snprintf(buf_for_fd2, sizeof(buf_for_fd2), "%s%s", wf, first_slash);
					fd_2 = open(buf_for_f2, O_WRONLY | O_TRUNC | O_CREAT, 0644);
				}
				def_file_copy(fd1, fd2, temp_buf);
			}
		}
		c++;

		if (a == NULL) {
			return 1;
		} else {
				if (rd->d_type == DT_DIR) {
				a++;
				index = strlen(temp_buf);
				snprintf(path + index, "/%s", rd->d_name);
				}
				return folder_copy(path);
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
	char *wf = argv[optind + 1];

	if (recursive_flag) {
		snprintf(path, sizeof(path), rf);
		folder_copy(path);
	}

	close(*fd1);
	close(*fd2);
}












