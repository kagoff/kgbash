
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "kgbash.h"

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
    if(strncmp(cmd, PWD_STRING, sizeof(PWD_STRING))) {
        return true;
    }
    if(strncmp(cmd, CD_STRING, sizeof(CD_STRING))) {
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
 * Assumes cmd input defined as cmd[n_commands][string_size]
 */
bool separate_commands (char** cmd, char* string, size_t n_commands, size_t string_size) {

    // Sanity check
    if(!cmd || !string) {
        return false;
    }

    uint32_t string_idx = 0;
    uint32_t cmd_idx = 0;
    uint32_t cmd_string_idx = 0;

    // Gather commands
    while(cmd_idx < n_commands) {
        // TODO: free this!!
        cmd[cmd_idx] = malloc(string_size*sizeof(char));

        // For each command index, parse the string until a special token
        while(!is_white_space_or_null(string[string_idx]) &&
              string_idx < string_size && cmd_string_idx < string_size) {
            cmd[cmd_idx][cmd_string_idx++] = string[string_idx++];
        }

        // If reached null terminator, stop
        if(string[string_idx] == '\0') {
            if(cmd_idx < (n_commands-1)) {
                cmd[cmd_idx+1] = NULL;
                return true;
            } else {
                return false;
            }
        }

        // Skip over white space and keep gathering commands
        string_idx++;
        cmd_idx++;
        cmd_string_idx = 0;
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
    char *cmd[COMMAND_ARRAY_LEN];
    int retval;
    pid_t pid;

    do {
        // Reset user input
        memset(raw_input, 0, INPUT_ARRAY_LEN*sizeof(char));
        memset(cmd, 0, COMMAND_ARRAY_LEN*sizeof(char*));

        // Prompt user for input
        fprintf(stdout, "kgbash: ");

        // Gather user input data, and skip fork if empty
        if(!parse_raw_input(raw_input, INPUT_ARRAY_LEN)) {
            fprintf(stdout, "\n");
            continue;
        }

        // TODO: separate out the input into distinct command arguments
        if(!separate_commands((char**)cmd, raw_input, COMMAND_ARRAY_LEN,
                              INPUT_ARRAY_LEN)) {
            fprintf(stderr, "\nInvalid input: %s\n", raw_input);
            continue;
        }

        // Check for exit condition
        if(is_exit_string(cmd[0])){
            if(!active_jobs) {
                fprintf(stderr, "\nBye...\n");
                return EXIT_SUCCESS;
            } else {
                fprintf(stderr, "\nError: active jobs still running\n");
                continue;
            }
        }

        // If we run an internal command, execute and continue
        // TODO: eventually make this sleepable...
        if(run_internal_cmds(cmd[0])) {
            continue;
        }

        // DEBUG: Display the user's command
        fprintf(stdout, "%s\n", cmd[0]);

        pid = fork();
        if (pid == 0) {
            execvp(cmd[0], (char**)cmd);
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

        fprintf(stderr, "+ completed '%s %s' [%d]\n", cmd[0], cmd[1], retval);

    } while(1);

    return EXIT_SUCCESS;
}