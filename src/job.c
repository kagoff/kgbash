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

#include "input.h"
#include "redirect.h"
#include "cmd.h"
#include "job.h"

static inline bool
is_item_redirect_token(previous_item) {
    switch(previous_item) {
        case KGBASH_ITEM_TYPE_INPUT_REDIRECT:
        case KGBASH_ITEM_TYPE_OUTPUT_REDIRECT:
            return true;
        default:
            return false;
    }
}

static inline bool
is_item_pipe_or_empty(previous_item) {
    switch(previous_item) {
        case KGBASH_ITEM_TYPE_EMPTY:
        case KGBASH_ITEM_TYPE_PIPE:
            return true;
        default:
            return false;
    }
}

static inline bool
is_item_special_token_or_empty(previous_item) {
    switch(previous_item) {
        case KGBASH_ITEM_TYPE_EMPTY:
        case KGBASH_ITEM_TYPE_INPUT_REDIRECT:
        case KGBASH_ITEM_TYPE_OUTPUT_REDIRECT:
        case KGBASH_ITEM_TYPE_PIPE:
        case KGBASH_ITEM_TYPE_SLEEP:
            return true;
        default:
            return false;
    }
}

static inline void
assign_command(job_s *job, cmd_s *cmd, uint32_t cmd_idx, uint32_t argc) {
    cmd->args[argc] = NULL;
    cmd->argc = argc;
    job->cmds[cmd_idx] = cmd;
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

    cmd_s *cmd = NULL;

    char    item[INPUT_ARRAY_LEN];
    uint16_t str_idx  = 0;
    uint16_t arg_idx  = 0;
    uint16_t cmd_idx  = 0;
    uint16_t item_idx = 0;

    // Save the raw input for later
    memcpy(job->raw_input, string, INPUT_ARRAY_LEN);
    job->raw_input[INPUT_ARRAY_LEN-1] = '\0';

    // Initialize item types
    kgbash_item_type_e previous_item = KGBASH_ITEM_TYPE_EMPTY;
    kgbash_item_type_e item_type = KGBASH_ITEM_TYPE_CMD_OR_ARG;

    // Continue grabbing items and making decisions until input is emptied
    item_type = input_grab_next_item(string, &str_idx, item, &item_idx);
    while(item_type != KGBASH_ITEM_TYPE_EMPTY) {

        switch(item_type) {
            case KGBASH_ITEM_TYPE_CMD_OR_ARG:
                // Allocate memory for a new command first time this type
                // appears at beginning or after token
                if(is_item_pipe_or_empty(previous_item)) {
                    // Assign the command if reached the next one
                    if(previous_item != KGBASH_ITEM_TYPE_EMPTY) {
                        assign_command(job, cmd, cmd_idx, arg_idx);
                        cmd_idx++;
                    }
                    arg_idx = 0;
                    cmd = cmd_create();
                    job->num_cmds++;
                    (cmd->args)[arg_idx] = malloc(INPUT_ARRAY_LEN*sizeof(char));
                    memset((cmd->args)[arg_idx], 0, INPUT_ARRAY_LEN);
                    memcpy(cmd->args[arg_idx++], item, item_idx);
                }
                // After a redirect, grab the file and finish up
                else if(is_item_redirect_token(previous_item)) {
                    memcpy(job->file, item, item_idx);
                    assign_command(job, cmd, cmd_idx, arg_idx);
                    return KGBASH_RET_SUCCESS;
                }
                // Make sure we don't go over the arg limit
                else if(arg_idx < MAX_ARGS) {
                    (cmd->args)[arg_idx] = malloc(INPUT_ARRAY_LEN*sizeof(char));
                    memset((cmd->args)[arg_idx], 0, INPUT_ARRAY_LEN);
                    memcpy(cmd->args[arg_idx++], item, item_idx);
                } else {
                    (cmd->args)[MAX_ARGS] = NULL;
                    break;
                }
                break;

            case KGBASH_ITEM_TYPE_INPUT_REDIRECT:
                if(is_item_special_token_or_empty(previous_item)) {
                    return KGBASH_RET_MISLOCATED_INPUT_REDIRECT;
                }
                job->redirect_in = true;
                break;

            case KGBASH_ITEM_TYPE_OUTPUT_REDIRECT:
                if(is_item_special_token_or_empty(previous_item)) {
                    return KGBASH_RET_MISLOCATED_OUTPUT_REDIRECT;
                }
                job->redirect_out = true;
                break;

            case KGBASH_ITEM_TYPE_PIPE:
                if(is_item_special_token_or_empty(previous_item)) {
                    return KGBASH_RET_MISLOCATED_PIPE;
                }
                job->pipes++;
                break;

            case KGBASH_ITEM_TYPE_SLEEP:
                if(is_item_special_token_or_empty(previous_item)) {
                    return KGBASH_RET_MISLOCATED_SLEEP;
                }
                job->sleep = true;
                break;

            default:
                return KGBASH_RET_FAIL;
        }
        previous_item = item_type;
        item_type = input_grab_next_item(string, &str_idx, item, &item_idx);

        // Sleep cannot be followed by anything
        if(item_type != KGBASH_ITEM_TYPE_EMPTY &&
           previous_item == KGBASH_ITEM_TYPE_SLEEP) {
            return KGBASH_RET_MISLOCATED_SLEEP;
        }
    }

    // The last item cannot be a redirect token or pipe
    switch(previous_item) {
        case KGBASH_ITEM_TYPE_INPUT_REDIRECT:
            return KGBASH_RET_MISLOCATED_INPUT_REDIRECT;
        case KGBASH_ITEM_TYPE_OUTPUT_REDIRECT:
            return KGBASH_RET_MISLOCATED_OUTPUT_REDIRECT;
        case KGBASH_ITEM_TYPE_PIPE:
            return KGBASH_RET_MISLOCATED_PIPE;
        default:
            break;
    }

    assign_command(job, cmd, cmd_idx, arg_idx+1);
    return KGBASH_RET_SUCCESS;
}

static kgbash_result_e
job_run_pipes(job_s* job) {
    if(!job) {
        return KGBASH_RET_NULL_PARAM;
    }

    pid_t pid;
    pid_t child = 0;

    uint16_t cmd_idx = 0;

    int std_out = dup(STDOUT_FILENO);
    int std_in = dup(STDIN_FILENO);
    int fd_old[2], fd_new[2];

    if(job->redirect_in) {
        kgbash_result_e ret = redirect_file_in(job->file, &std_in);
        if(ret != KGBASH_RET_SUCCESS) {
            return ret;
        }
    }

    // Create the first child
    pipe(fd_old);
    pid = fork();
    if (pid != 0) {
        job->cmds[cmd_idx]->pid = pid;
    } else if (pid == 0) {
        close(fd_old[0]);
        dup2(fd_old[1], STDOUT_FILENO);
        close(fd_old[1]);

        execvp(job->cmds[cmd_idx]->args[0], job->cmds[cmd_idx]->args);
        exit(EXIT_FAILURE);
    } else {
        // Should never happen
        exit(EXIT_FAILURE);
    }

    // Continue creating children and piping I/O
    for(uint16_t pipes_left = job->pipes; pipes_left > 0; pipes_left--) {
        // Check the index before dereferencing
        cmd_idx = ((job->pipes) - pipes_left) + 1;
        if(!job->cmds[cmd_idx]) {
            return KGBASH_RET_FAIL;
        }

        if(pipes_left == 1 && job->redirect_out) {
            kgbash_result_e ret = redirect_file_out(job->file, &std_out);
            if(ret != KGBASH_RET_SUCCESS) {
                return ret;
            }
        }

        pipe(fd_new);
        pid = fork();
        if (pid != 0) {
            job->cmds[cmd_idx]->pid = pid;
            close(fd_old[0]);
            close(fd_old[1]);
            if(pipes_left > 1) {
                fd_old[0] = fd_new[0];
                fd_old[1] = fd_new[1];
            } else {
                close(fd_new[0]);
                close(fd_new[1]);
            }
        } else if (pid == 0) {
            // Always set the input from previous child
            close(fd_old[1]);
            dup2(fd_old[0], STDIN_FILENO);
            close(fd_old[0]);

            // If there are more pipes remaining, also set output
            if(pipes_left > 1) {
                close(fd_new[0]);
                dup2(fd_new[1], STDOUT_FILENO);
                close(fd_new[1]);
            }

            execvp(job->cmds[cmd_idx]->args[0], job->cmds[cmd_idx]->args);
            exit(EXIT_FAILURE);
        } else {
            // Should never happen
            exit(EXIT_FAILURE);
        }
    }
    // Collect all the commands
    for(uint16_t pipes = (job->pipes + 1); pipes > 0; pipes--) {
        uint16_t cmd_idx = pipes - 1;
        fprintf(stderr, "Collecting cmd=%d, pid=%d\n", cmd_idx, job->cmds[cmd_idx]->pid);
        waitpid(job->cmds[cmd_idx]->pid, &job->cmds[cmd_idx]->retval, 0);
        fprintf(stderr, "Collected  cmd=%d, pid=%d\n", cmd_idx, job->cmds[cmd_idx]->pid);
    }

    redirect_reset_file_descriptors(std_in, std_out);
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