
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "kgbash.h"
#include "cmd.h"

static bool active_jobs = false;

bool is_exit_string(char* cmd) {
    if(cmd && !strncmp(cmd, EXIT_STRING, sizeof(EXIT_STRING)-1)) {
        return true;
    }
    return false;
}

// TODO: write this
bool run_internal_cmds(char* cmd) {
    if(!cmd) {
        return false;
    }
    if(!strncmp(cmd, PWD_STRING, sizeof(PWD_STRING))) {
        return true;
    }
    if(!strncmp(cmd, CD_STRING, sizeof(CD_STRING))) {
        return true;
    }
    return false;
}

// TODO: write this
bool is_special_token(char token) {
    return false;
}

bool is_white_space_or_null(char token) {
    if(token == ' ' || token == '\t' || token == '\0') {
        return true;
    }
    return false;
}

/*
 * Assumes
 */
bool separate_commands (cmd_s *cmd, char* string, size_t n_args, size_t string_size) {

    // Sanity check
    if(!cmd || !string) {
        return false;
    }

    uint32_t string_idx = 0;
    uint32_t arg_idx = 0;
    uint32_t cmd_string_idx = 0;

    // Get the name of the command to run
    while(!is_white_space_or_null(string[string_idx])
        && string_idx < string_size && cmd_string_idx < string_size) {
            (cmd->command)[cmd_string_idx++] = string[string_idx++];
    }
    // TODO: check boundaries here
    (cmd->command)[cmd_string_idx] = '\0';
    (cmd->args)[arg_idx] = (char*)&(cmd->command);

    // If reached null terminator, stop
    if(string[string_idx] == '\0') {
        if(arg_idx < (n_args-1)) {
            (cmd->args)[arg_idx+1] = NULL;
            return true;
        } else {
            return false;
        }
    }

    // Gather args
    while(arg_idx < n_args) {
        cmd_string_idx = 0;
        arg_idx++; //increment first since we already set first arg to the cmd

        // Skip over white space 
        while(is_white_space_or_null(string[string_idx])) {
            string_idx++;
        }

        // TODO: free this!!
        (cmd->args)[arg_idx] = malloc(string_size*sizeof(char));

        // For each command index, parse the string until a special token
        while(!is_white_space_or_null(string[string_idx]) &&
              string_idx < string_size && cmd_string_idx < string_size) {
            (cmd->args)[arg_idx][cmd_string_idx++] = string[string_idx++];
        }

        // If reached null terminator, stop
        if(string[string_idx] == '\0') {
            if(arg_idx < (n_args-1)) {
                (cmd->args)[arg_idx+1] = NULL;
                return true;
            } else {
                return false;
            }
        }
    }
    return true;
}

/*
 * Fills a string to contain only the null-terminated user input without
 * a trailing endline character.
 * 
 * Returns:
 *  - False if input is empty
 *  - True otherwise
 */
 bool parse_raw_input(char * string, size_t string_size) {
    uint32_t string_idx = 0;

    // Collect user input
    do {
        string[string_idx] = getchar();
    } while(string[string_idx++] != '\n' &&
            string_idx < string_size);

    // Return false on empty user input
    if(string[0] == '\n') {
        return false;
    }

    // Clear input buffer if needed
    if(string[string_idx-1] != '\n') {
        while(getchar() != '\n');
        string[string_idx] = '\0';
    } else {
        string[string_idx-1] = '\0';
    }

    return true;
}

int main() {

    char raw_input[INPUT_ARRAY_LEN];
    cmd_s *cmd = malloc(sizeof(cmd_s));
    int retval;
    pid_t pid;

    do {
        // Reset user input
        memset(raw_input, 0, INPUT_ARRAY_LEN*sizeof(char));
        memset(cmd, 0, sizeof(cmd_s));

        // Prompt user for input
        fprintf(stdout, "kgbash: ");

        // Gather user input data, and skip fork if empty
        if(!parse_raw_input(raw_input, INPUT_ARRAY_LEN)) {
            fprintf(stdout, "\n");
            continue;
        }

        // TODO: separate out the input into distinct command arguments
        if(!separate_commands(cmd, raw_input, ARG_ARRAY_LEN,
                              INPUT_ARRAY_LEN)) {
            fprintf(stderr, "Invalid input: %s\n", raw_input);
            continue;
        }

        // Check for exit condition
        if(is_exit_string((char*)(cmd->command))) {
            if(!active_jobs) {
                fprintf(stderr, "Bye...\n");
                return EXIT_SUCCESS;
            } else {
                fprintf(stderr, "Error: active jobs still running\n");
                continue;
            }
        }

        // If we run an internal command, execute and continue
        // TODO: eventually make this sleepable...
        if(run_internal_cmds((char*)(cmd->command))) {
            continue;
        }

        // DEBUG: Display the user's command
        fprintf(stdout, "%s\n", cmd->command);

        pid = fork();
        if (pid == 0) {
            execvp(cmd->command, cmd->args);
            exit(EXIT_FAILURE);
        }
        else if (pid > 0) {
            wait(&retval);
            // TODO: retrieve this retval correctly
            retval = EXIT_SUCCESS;
        }
        else {
            exit(EXIT_FAILURE);
        }

        fprintf(stderr, "+ completed '%s %s' [%d]\n", cmd->command, (cmd->args)[1], retval);

    } while(1);

    return EXIT_SUCCESS;
}