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
char buf[PATH_MAX];
char path[PATH_MAX];
char temp_path[PATH_MAX] = {0};
char* first_slash;
char* wf;
int fd_1, fd_2;
int* fd1 = &fd_1;
int* fd2 = &fd_2;
int dir_fd;
int dir_fd2;
DIR* dir;
DIR* dir2;
DIR* temp_dir;
struct dirent* global_rd = {0};
struct dirent* rd = {0};
size_t pindex = 0;
bool is_folder = false;
bool in_process = true;

void def_files_copy(DIR* dir, DIR* dir2, char* temp_buf) {
        struct dirent* rd = {0};
        while ((rd = readdir(dir)) != NULL) {
                if (strcmp(rd->d_name, ".") == 0 || strcmp(rd->d_name, "..") == 0) {
                continue;
                }
                if (rd->d_type != DT_DIR) {
                dir_fd = dirfd(dir);
                fd_1 = openat(dir_fd, rd->d_name, O_RDONLY);
                if (fd_1 == - 1) {
                        perror("fd_1 error");
                }
                dir_fd2 = dirfd(dir2);
                fd_2 = openat(dir_fd2, rd->d_name, O_WRONLY | O_TRUNC | O_CREAT, 0644);
                if (fd_2 == - 1) {
                        perror("fd_2 error");
                }

                ssize_t len = 0;
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
        close(*fd2);
        close(*fd1);
}

void folder_copy(char* path) {

        is_folder = false;

        while ((global_rd = readdir(dir)) != NULL && !in_process) {
                if (strcmp(global_rd->d_name, ".") == 0 || strcmp(global_rd->d_name, "..") == 0)
                continue;
                if (global_rd->d_type == DT_DIR) {
                        snprintf(path + strlen(path), sizeof(path), "/%s", global_rd->d_name);
                        first_slash = strchr(buf_for_fd2, '/');
                        *first_slash = '\0';
                        snprintf(buf_for_fd2 + strlen(buf_for_fd2), sizeof(buf_for_fd2), "/%s", global_rd->d_name);

                        if (mkdir(path, 0777) == -1 ) {
                                if (errno != EEXIST) {
                                perror("CAN'T CREATE FOLDER\n");
                                }
                        } else {
                                fprintf(stderr, "FOLDER FOR PATH %s CREATED!\n", path);
                        }

                        if (mkdir(buf_for_fd2, 0777) == -1 ) {
                                if (errno != EEXIST) {
                                perror("CAN'T CREATE FOLDER\n");
                                }
                        } else {
                                fprintf(stderr, "FOLDER FOR BUF_FOR_FD_2 %s CREATED!\n", buf_for_fd2);
                        }
                }
        }

        if (mkdir(path, 0777) == -1 ) {
                if (errno != EEXIST) {
                        perror("CAN'T CREATE FOLDER\n");
                }
                } else {
                        fprintf(stderr, "FOLDER FOR PATH %s CREATED!\n", path);
                }

                if (mkdir(buf_for_fd2, 0777) == -1 ) {
                        if (errno != EEXIST) {
                        perror("CAN'T CREATE FOLDER\n");
                        }
                } else {
                        fprintf(stderr, "FOLDER FOR BUF_FOR_FD_2 %s CREATED!\n", buf_for_fd2);
        }


        temp_dir = opendir(path);
        dir2 = opendir(buf_for_fd2);

        def_files_copy(temp_dir, dir2, temp_buf);
        rewinddir(temp_dir);

        if (global_rd == NULL && !is_folder) {
                exit(EXIT_SUCCESS);
        } else {
                while ((rd = readdir(temp_dir)) != NULL) {
                        if (strcmp(rd->d_name, ".") == 0 || strcmp(rd->d_name, "..") == 0)
                        continue;
                        if (rd->d_type == DT_DIR) {
                                snprintf(path + strlen(path), sizeof(path), "/%s", rd->d_name);
                                snprintf(buf_for_fd2 + strlen(buf_for_fd2), sizeof(buf_for_fd2), "/%s", rd->d_name);
                                in_process = true;
                                folder_copy(path);
                        }
                }
                in_process = false;
                first_slash = strchr(path, '/');
                *first_slash = '\0';
                folder_copy(path);
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
                snprintf(path, sizeof(temp_path), "%s", rf);
                snprintf(buf_for_fd2, sizeof(buf_for_fd2), wf);
                dir = opendir(path);
                folder_copy(path);
        }

        closedir(dir);
        closedir(dir2);
        closedir(temp_dir);
}

