#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "definitions.h"
#include "types.h"

#include "input.h"

// TODO: write this
static bool is_special_token(char token) {
    return false;
}

static bool is_white_space_or_null(char token) {
    if(token == ' ' || token == '\t' || token == '\0') {
        return true;
    }
    return false;
}

bool input_is_exit_string(const char* cmd) {
    if(cmd && !strncmp(cmd, EXIT_STRING, sizeof(EXIT_STRING)-1)) {
        return true;
    }
    return false;
}

/*
 * Assumes
 */
bool input_separate_commands (cmd_s *cmd, const char* string, size_t n_args, size_t string_size) {

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
 bool input_parse_raw_input(char * string, size_t string_size) {
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