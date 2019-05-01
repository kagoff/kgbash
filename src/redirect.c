#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>

#include "definitions.h"
#include "types.h"

#include "redirect.h"

kgbash_result_e
redirect_file_out(const char* file, int* stdout_fd) {
    if(!file || !stdout_fd) {
        return KGBASH_RET_NULL_PARAM;
    }
    // Make a copy of stdout to return
    *stdout_fd = dup(STDOUT_FILENO);

    // Open the given file for writing
    int fd;
    fd = open(file, O_WRONLY);
    if(fd < 0) {
        return KGBASH_RET_FAIL;
    }

    // Switch the output and close extra fd
    dup2(fd, STDOUT_FILENO);
    close(fd);
    return KGBASH_RET_SUCCESS;
}

kgbash_result_e
redirect_file_in(const char* file, int* stdin_fd) {
    if(!file || !stdin_fd) {
        return KGBASH_RET_NULL_PARAM;
    }
    // Make a copy of stdout to return
    *stdin_fd = dup(STDIN_FILENO);

    // Open the given file for reading
    int fd;
    fd = open(file, O_RDONLY);
    if(fd < 0) {
        return KGBASH_RET_FAIL;
    }

    // Switch the output and close extra fd
    dup2(fd, STDIN_FILENO);
    close(fd);
    return KGBASH_RET_SUCCESS;
}

void
redirect_reset_file_descriptors(int stdin_fd, int stdout_fd) {
    if(stdin_fd >= 0) {
        dup2(stdin_fd, STDIN_FILENO);
        close(stdin_fd);
    }
    if(stdout_fd >= 0) {
        dup2(stdout_fd, STDOUT_FILENO);
        close(stdout_fd);
    }
}