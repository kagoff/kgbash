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
static inline void clear_leading_whitespace(const char* string, uint32_t *str_idx) {
    while(is_white_space(string[*str_idx])) {
        (*str_idx)++;
    }
}
static inline void assign_command(job_s *job, cmd_s *cmd, uint32_t cmd_idx, uint32_t argc) {
    cmd->args[argc] = NULL;
    cmd->argc = argc;
    job->cmds[cmd_idx] = cmd;
}
static inline kgbash_result_e misplaced_token_return_value(char token) {
    switch(token) {
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

job_s *job_create(void) {
    job_s *job = malloc(sizeof(job_s));
    for(uint32_t cmd_idx = 0; cmd_idx < INPUT_ARRAY_LEN; cmd_idx++) {
        job->cmds[cmd_idx] = NULL;
    }
    memset(job->raw_input, 0, INPUT_ARRAY_LEN);
    memset(job->file, 0, INPUT_ARRAY_LEN);
    job->num_cmds = 0;
    job->pipes = 0;
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
    uint32_t cmd_idx = 0;
    bool piped = false;

    // Save the raw input for later
    memcpy(job->raw_input, string, INPUT_ARRAY_LEN);
    job->raw_input[INPUT_ARRAY_LEN-1] = '\0';

    // Called in between every found arg or token
    clear_leading_whitespace(string, &str_idx);

    // Return emtpy error if NULL found first
    if(is_null(string[str_idx])) {
        return KGBASH_RET_EMPTY_INPUT;
    }

    // Go through each command until pipes are found
    while(1) {
        // Return if token found first, or just after a pipe
        clear_leading_whitespace(string, &str_idx);
        if(is_special_token(string[str_idx])) {
            return misplaced_token_return_value(string[str_idx]);
        }
        // Allocate stack memory (freed by cmd_free)
        cmd_s *cmd = cmd_create();
        job->num_cmds++;
        // TODO: make function for this
        (cmd->args)[arg_idx] = malloc(INPUT_ARRAY_LEN*sizeof(char));

        // Get the name of the command to run
        while(!is_white_space_or_null(string[str_idx]) &&
              str_idx < INPUT_ARRAY_LEN &&
              cmd_str_idx < INPUT_ARRAY_LEN) {
            cmd->args[arg_idx][cmd_str_idx++] = string[str_idx++];
        }
        clear_leading_whitespace(string, &str_idx);

        // If reached null terminator, stop
        if(string[str_idx] == '\0') {
            assign_command(job, cmd, cmd_idx, arg_idx+1);
            return KGBASH_RET_SUCCESS;
        }

        // Gather args
        piped = false;
        while(!piped && arg_idx < INPUT_ARRAY_LEN-1) {
            cmd_str_idx = 0;
            arg_idx++; //increment first since we already set first arg to the cmd

            // Skip over white space 
            while(is_white_space_or_null(string[str_idx])) {
                str_idx++;
            }

            // Check for redirect
            if(is_output_redirect(string[str_idx])) {
                // TODO: check command and arg index to see if in wrong spot
                job->redirect_out = true;
                assign_command(job, cmd, cmd_idx, arg_idx);
                str_idx++;
            } else if(is_input_redirect(string[str_idx])) {
                // TODO: check arg_idx and cmd_idx to see if in wrong spot
                job->redirect_in = true;
                assign_command(job, cmd, cmd_idx, arg_idx);
                str_idx++;
            } else if (is_pipe(string[str_idx])) {
                piped = true;
                job->pipes++;
                // assign_command(job, cmd, cmd_idx, arg_idx);
                str_idx++;
                break;
            }
            clear_leading_whitespace(string, &str_idx);

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

            // TODO: check for misplaced redirect and for extra characters!
            if(job->redirect_out || job->redirect_in) {
                return KGBASH_RET_SUCCESS;
            }
            clear_leading_whitespace(string, &str_idx);

            // If reached null terminator, stop
            if(string[str_idx] == '\0') {
                assign_command(job, cmd, cmd_idx, arg_idx+1);
                return KGBASH_RET_SUCCESS;
            }
        }
        // User gave too many arguments, overwrite the last one with NULL
        assign_command(job, cmd, cmd_idx, arg_idx);

        // Rare case of end of arg array on last command
        if(!piped) {
            return KGBASH_RET_SUCCESS;
        }

        // Move to the next command
        cmd_idx++;
    }
}

static kgbash_result_e
job_run_pipes(job_s* job) {
    if(!job) {
        return KGBASH_RET_NULL_PARAM;
    }

    pid_t pid;
    int fd[2];
    uint8_t cmd_idx = 0;

    for(uint8_t pipes_left = job->pipes; pipes_left > 0; pipes_left--) {
        // Check the index before dereferencing
        cmd_idx = (job->pipes) - pipes_left;
        if(!job->cmds[cmd_idx]) {
            exit(EXIT_FAILURE);
        }

        pipe(fd);
        pid = fork();
        if (pid != 0) {
            // Parent writes to pipe output
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);
            execvp(job->cmds[cmd_idx]->args[0], job->cmds[cmd_idx]->args);
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child waits for parent to finish, then reads from pipe input
            close(fd[1]);
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);
            waitpid(pid, &job->cmds[cmd_idx]->retval, 0);
            if(WEXITSTATUS(job->cmds[cmd_idx]->retval) == EXIT_FAILURE) {
                exit(EXIT_FAILURE);
            }
        } else {
            // Should never happen
            exit(EXIT_FAILURE);
        }
    }

    return KGBASH_RET_SUCCESS;
}

kgbash_result_e
job_run(job_s *job) {
    pid_t pid;
    int stdin_fd = -1;
    int stdout_fd = -1;
    kgbash_result_e ret;

    if(job->pipes > 0) {
        return job_run_pipes(job);
    }

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
        job->cmds[0]->retval = EXIT_SUCCESS;
        redirect_reset_file_descriptors(stdin_fd, stdout_fd);
        return KGBASH_RET_SUCCESS;
    }

    pid = fork();
    if (pid == 0) {
        execvp(job->cmds[0]->args[0], job->cmds[0]->args);
        exit(EXIT_FAILURE);
    }
    else if (pid > 0) {
        if(!job->sleep) {
            waitpid(pid, &job->cmds[0]->retval, 0);
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