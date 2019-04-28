#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "definitions.h"
#include "types.h"

#include "job.h"

// TODO: write this
static inline bool is_special_token(char token) {
    return false;
}

static inline bool is_white_space_or_null(char token) {
    if(token == ' ' || token == '\t' || token == '\0') {
        return true;
    }
    return false;
}

//TODO: write this
void job_free(job_s **job) {
    return;
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

    uint32_t string_idx = 0;
    uint32_t arg_idx = 0;
    uint32_t cmd_string_idx = 0;
    uint32_t n_cmds = 0;

    cmd_s *cmd = malloc(sizeof(cmd_s));
    job->cmds = malloc(sizeof(cmd_s*));

    // Get the name of the command to run
    while(!is_white_space_or_null(string[string_idx])
        && string_idx < INPUT_ARRAY_LEN && cmd_string_idx < INPUT_ARRAY_LEN) {
            (cmd->command)[cmd_string_idx++] = string[string_idx++];
    }
    // TODO: check boundaries here
    (cmd->command)[cmd_string_idx] = '\0';
    (cmd->args)[arg_idx] = (char*)&(cmd->command);

    // If reached null terminator, stop
    if(string[string_idx] == '\0') {
        if(arg_idx < (ARG_ARRAY_LEN-1)) {
            (cmd->args)[arg_idx+1] = NULL;
            job->cmds[0] = cmd;
            return true;
        } else {
            return false;
        }
    }

    // Gather args
    while(arg_idx < INPUT_ARRAY_LEN-1) {
        cmd_string_idx = 0;
        arg_idx++; //increment first since we already set first arg to the cmd

        // Skip over white space 
        while(is_white_space_or_null(string[string_idx])) {
            string_idx++;
        }

        // TODO: free this!!
        (cmd->args)[arg_idx] = malloc(INPUT_ARRAY_LEN*sizeof(char));

        // For each command index, parse the string until a special token
        while(!is_white_space_or_null(string[string_idx]) &&
              string_idx < INPUT_ARRAY_LEN && cmd_string_idx < INPUT_ARRAY_LEN) {
            (cmd->args)[arg_idx][cmd_string_idx++] = string[string_idx++];
        }

        // If reached null terminator, stop
        if(string[string_idx] == '\0') {
            if(arg_idx < (ARG_ARRAY_LEN-1)) {
                (cmd->args)[arg_idx+1] = NULL;
                job->cmds[0] = cmd;
                return true;
            } else {
                return false;
            }
        }
    }
    job->cmds[0] = cmd;
    return true;
}

// TODO: write this
bool job_run_internal(const job_s* job) {
    if(!job || !job->cmds[0]) {
        return false;
    }
    const char* cmd = job->cmds[0]->command;

    if(!strncmp(cmd, PWD_STRING, sizeof(PWD_STRING))) {
        return true;
    }
    if(!strncmp(cmd, CD_STRING, sizeof(CD_STRING))) {
        return true;
    }
    return false;
}