#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "definitions.h"
#include "types.h"

#include "redirect.h"
#include "cmd.h"
#include "job.h"

// Local character checking functions
static inline bool is_output_redirect(char token) {
    return token == '>' ? true : false;
}
static inline bool is_input_redirect(char token) {
    return token == '<' ? true : false;
}
static inline bool is_pipe(char token) {
    return token == '|' ? true : false;
}
static inline bool is_null(char token) {
    return token == '\0' ? true: false;
}
static inline bool is_white_space(char token) {
    if(token == ' ' || token == '\t') {
        return true;
    }
    return false;
}
static inline bool is_white_space_or_null(char token) {
    switch(token) {
        case ' ':
        case '\t':
        case '\0':
            return true;
        default:
            return false;
    }
}
static inline bool is_special_token(char token) {
    switch(token) {
        case '|':
        case '<':
        case '>':
            return true;
        default:
            return false;
    }
}

job_s *job_create(void) {
    job_s *job = malloc(sizeof(job_s));
    for(uint32_t cmd_idx = 0; cmd_idx < INPUT_ARRAY_LEN; cmd_idx++) {
        job->cmds[cmd_idx] = NULL;
        job->retvals[cmd_idx] = 0;
    }
    job->pipe_to_next = NULL;
    job->redirect_in = false;
    job->redirect_out = false;
    job->sleep = false;
    return job;
}

void job_free(job_s **job_ptr) {
    if(!job_ptr || !(*job_ptr)) {
        return;
    }
    job_s *job = *job_ptr;

    uint32_t cmd_idx = 0;
    while((job->cmds)[cmd_idx]) {
        cmd_free(&((job->cmds)[cmd_idx++]));
    }

    if(job->pipe_to_next) {
        free(job->pipe_to_next);
    }

    // Clear job pointer
    free(job);
    job = NULL;
}

bool job_is_exit_string(const job_s* job) {
    if(job && job->cmds[0] && job->cmds[0]->args[0]) {
        const char* cmd = job->cmds[0]->args[0];
        if(!strncmp(cmd, EXIT_STRING, sizeof(EXIT_STRING)-1)) {
            return true;
        }
    }
    return false;
}

kgbash_result_e
job_fill_from_input (job_s* job, const char* string) {

    // Sanity check
    if(!job || !string) {
        return KGBASH_RET_NULL_PARAM;
    }

    uint32_t str_idx = 0;
    uint32_t arg_idx = 0;
    uint32_t cmd_str_idx = 0;
    uint32_t n_cmds = 0;

    cmd_s *cmd = cmd_create();

    // Skip leading white space and return if nothing found
    while(is_white_space(string[str_idx])) {
        str_idx++;
    }
    if(is_null(string[str_idx])) {
        return KGBASH_RET_EMPTY_INPUT;
    } else if(is_special_token(string[str_idx])) {
        switch(string[str_idx]) {
            case '<':
                return KGBASH_RET_MISPLACED_INPUT_REDIRECT;
                break;
            case '>':
                return KGBASH_RET_MISPLACED_OUTPUT_REDIRECT;
                break;
            case '|':
                return KGBASH_RET_MISPLACED_PIPE;
                break;
            default:
                return KGBASH_RET_FAIL;
        }
    }

    // Get the name of the command to run
    (cmd->args)[arg_idx] = malloc(INPUT_ARRAY_LEN*sizeof(char));
    while(!is_white_space_or_null(string[str_idx]) &&
          str_idx < INPUT_ARRAY_LEN &&
          cmd_str_idx < INPUT_ARRAY_LEN) {
        cmd->args[arg_idx][cmd_str_idx++] = string[str_idx++];
    }

    // If reached null terminator, stop
    if(string[str_idx] == '\0') {
        cmd->argc = arg_idx+1;
        cmd->args[arg_idx+1] = NULL;
        job->cmds[0] = cmd;
        return KGBASH_RET_SUCCESS;
    }

    // Gather args
    while(arg_idx < INPUT_ARRAY_LEN-1) {
        cmd_str_idx = 0;
        arg_idx++; //increment first since we already set first arg to the cmd

        // Skip over white space 
        while(is_white_space_or_null(string[str_idx])) {
            str_idx++;
        }

        // Check for redirect
        if(is_output_redirect(string[str_idx])) {
            job->redirect_out = true;
            cmd->args[arg_idx] = NULL;
            cmd->argc = arg_idx;
            job->cmds[0] = cmd;
            str_idx++;
        } else if(is_input_redirect(string[str_idx])) {
            job->redirect_in = true;
            cmd->args[arg_idx] = NULL;
            cmd->argc = arg_idx;
            job->cmds[0] = cmd;
            str_idx++;
        }
        // TODO: check for pipes

        while(is_white_space(string[str_idx])) {
            str_idx++;
        }

        // If redirecting, there should be no more args
        if(!job->redirect_out && !job->redirect_in) {
            cmd->args[arg_idx] = malloc(INPUT_ARRAY_LEN*sizeof(char));
        }

        // For each command index, parse the string until a special token
        while(!is_white_space_or_null(string[str_idx]) &&
              str_idx < INPUT_ARRAY_LEN && cmd_str_idx < INPUT_ARRAY_LEN) {
            // If args still being collected, fill arg
            if(!job->redirect_out && !job->redirect_in) {
                cmd->args[arg_idx][cmd_str_idx++] = string[str_idx++];
            // Otherwise, fill the file name
            } else {
                job->file[cmd_str_idx++] = string[str_idx++];
            }
        }

        // TODO: check for misplaced redirect!
        if(job->redirect_out || job->redirect_in) {
            return KGBASH_RET_SUCCESS;
        }

        while(is_white_space(string[str_idx])) {
            str_idx++;
        }

        // If reached null terminator, stop
        if(string[str_idx] == '\0') {
            cmd->args[arg_idx+1] = NULL;
            cmd->argc = arg_idx+1;
            job->cmds[0] = cmd;
            return KGBASH_RET_SUCCESS;
        }
    }
    // User gave too many arguments, overwrite the last one with NULL
    cmd->args[arg_idx] = NULL;
    cmd->argc = arg_idx;
    job->cmds[0] = cmd;
    return KGBASH_RET_SUCCESS;
}

kgbash_result_e
job_run(job_s *job, bool sleep) {
    pid_t pid;
    int stdin_fd = -1;
    int stdout_fd = -1;
    kgbash_result_e ret;

    //TODO: check return values
    if(job->redirect_out) {
        ret = redirect_file_out(job->file, &stdout_fd);
        if(ret != KGBASH_RET_SUCCESS) {
            return ret;
        }
    } else if(job->redirect_in) {
        ret = redirect_file_in(job->file, &stdin_fd);
        if(ret != KGBASH_RET_SUCCESS) {
            return ret;
        }
    }

    if(job_run_internal(job)) {
        job->retvals[0] = EXIT_SUCCESS;
        redirect_reset_file_descriptors(stdin_fd, stdout_fd);
        return KGBASH_RET_SUCCESS;
    }

    pid = fork();
    if (pid == 0) {
        execvp(job->cmds[0]->args[0], job->cmds[0]->args);
        exit(EXIT_FAILURE);
    }
    else if (pid > 0) {
        if(!sleep) {
            wait(&job->retvals[0]);
            // TODO: retrieve this retval correctly
            job->retvals[0] = EXIT_SUCCESS;
        } else {
            // TODO: enqueue this pid to check later
            return KGBASH_RET_SUCCESS;
        }
    }
    else {
        exit(EXIT_FAILURE);
    }

    redirect_reset_file_descriptors(stdin_fd, stdout_fd);
    return KGBASH_RET_SUCCESS;
}

bool job_run_internal(job_s* job) {
    if(!job || !job->cmds[0] || !(job->cmds[0]->args)[0]) {
        return false;
    }
    const char* cmd = (job->cmds[0]->args)[0];
    char cur_dir[PATH_MAX];

    if(!strncmp(cmd, PWD_STRING, sizeof(PWD_STRING))) {
        // TODO: this could be piped
        fprintf(stdout, "%s\n", getcwd(cur_dir, PATH_MAX));
        return true;
    }
    if(!strncmp(cmd, CD_STRING, sizeof(CD_STRING))) {
        chdir((job->cmds[0]->args)[1]);
        return true;
    }
    return false;
}