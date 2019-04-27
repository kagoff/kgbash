
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "definitions.h"
#include "types.h"
#include "input.h"
#include "cmds.h"

#include "kgbash.h"

static bool active_jobs = false;

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
        if(!input_parse_raw_input(raw_input, INPUT_ARRAY_LEN)) {
            fprintf(stdout, "\n");
            continue;
        }

        // TODO: separate out the input into distinct command arguments
        if(!input_separate_commands(cmd, raw_input, ARG_ARRAY_LEN,
                              INPUT_ARRAY_LEN)) {
            fprintf(stderr, "Invalid input: %s\n", raw_input);
            continue;
        }

        // Check for exit condition
        if(input_is_exit_string((char*)(cmd->command))) {
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
        if(cmds_run_internal((const char*)(cmd->command))) {
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