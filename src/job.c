#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

#include "definitions.h"
#include "types.h"

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
    if(is_white_space(token) || is_null(token)) {
        return true;
    }
    return false;
}

job_s *job_create(void) {
    job_s *job = malloc(sizeof(job_s));
    for(uint32_t cmd_idx = 0; cmd_idx < INPUT_ARRAY_LEN; cmd_idx++) {
        job->cmds[cmd_idx] = NULL;
    }
    job->pipe_to_next = NULL;
    job->redirect_in = false;
    job->redirect_out = false;
    job->sleep = false;
    return job;
}

void job_free(job_s *job) {
    if(!job) {
        return;
    }
    uint32_t cmd_idx = 0;
    while((job->cmds)[cmd_idx]) {
        cmd_free((job->cmds)[cmd_idx++]);
    }
    free(job->cmds);

    if(job->pipe_to_next) {
        free(job->pipe_to_next);
    }

    // Clear job pointer
    free(job);
}

bool job_is_exit_string(const job_s* job) {
    if(job && job->cmds[0]) {
        const char* cmd = job->cmds[0]->command;
        if(!strncmp(cmd, EXIT_STRING, sizeof(EXIT_STRING)-1)) {
            return true;
        }
    }
    return false;
}

/*
 * Assumes
 */
bool job_fill_from_input (job_s* job, const char* string) {

    // Sanity check
    if(!job || !string) {
        return false;
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
        return false;
    }

    // Get the name of the command to run
    while(!is_white_space_or_null(string[str_idx]) &&
          str_idx < INPUT_ARRAY_LEN &&
          cmd_str_idx < INPUT_ARRAY_LEN) {
        (cmd->command)[cmd_str_idx++] = string[str_idx++];
    }
    // TODO: check boundaries here
    (cmd->command)[cmd_str_idx] = '\0';
    (cmd->args)[arg_idx] = (char*)&(cmd->command);

    // If reached null terminator, stop
    if(string[str_idx] == '\0') {
        if(arg_idx < (ARG_ARRAY_LEN-1)) {
            (cmd->args)[arg_idx+1] = NULL;
            (cmd->argc) = arg_idx+1;
            job->cmds[0] = cmd;
            return true;
        } else {
            return false;
        }
    }

    // Gather args
    while(arg_idx < INPUT_ARRAY_LEN-1) {
        cmd_str_idx = 0;
        arg_idx++; //increment first since we already set first arg to the cmd

        // Skip over white space 
        while(is_white_space_or_null(string[str_idx])) {
            str_idx++;
        }

        (cmd->args)[arg_idx] = malloc(INPUT_ARRAY_LEN*sizeof(char));

        // For each command index, parse the string until a special token
        while(!is_white_space_or_null(string[str_idx]) &&
              str_idx < INPUT_ARRAY_LEN && cmd_str_idx < INPUT_ARRAY_LEN) {
            (cmd->args)[arg_idx][cmd_str_idx++] = string[str_idx++];
        }

        // If reached null terminator, stop
        if(string[str_idx] == '\0') {
            if(arg_idx < (ARG_ARRAY_LEN-1)) {
                (cmd->args)[arg_idx+1] = NULL;
                (cmd->argc) = arg_idx+1;
                job->cmds[0] = cmd;
                return true;
            } else {
                return false;
            }
        }
    }
    (cmd->argc) = arg_idx;
    job->cmds[0] = cmd;
    return true;
}

// TODO: write this
bool job_run_internal(const job_s* job) {
    if(!job || !job->cmds[0]) {
        return false;
    }
    const char* cmd = job->cmds[0]->command;
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