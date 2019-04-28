
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
#include "job.h"

#include "kgbash.h"

static bool active_jobs = false;
// TODO: have job queue here

int main() {

    char input[INPUT_ARRAY_LEN];
    job_s *job;
    int retval;
    pid_t pid;

    do {
        // Reset user input
        memset(input, 0, INPUT_ARRAY_LEN*sizeof(char));

        // Prompt user for input
        fprintf(stdout, "kgbash: ");

        // Gather user input data, and skip fork if empty
        if(!input_parse_input(input, INPUT_ARRAY_LEN)) {
            fprintf(stdout, "\n");
            continue;
        }

        // TODO: separate out the input into distinct command arguments
        job = malloc(sizeof(job_s));
        if(!job_fill_from_input(job, input)) {
            fprintf(stderr, "Invalid input: %s\n", input);
            job_free(&job);
            continue;
        }

        // Check for exit condition
        if(job_is_exit_string(job)) {
            if(!active_jobs) {
                fprintf(stderr, "Bye...\n");
                job_free(&job);
                return EXIT_SUCCESS;
            } else {
                fprintf(stderr, "Error: active jobs still running\n");
                job_free(&job);
                continue;
            }
        }

        // If we run an internal command, execute and continue
        // TODO: eventually make this sleepable...
        if(job_run_internal(job)) {
            job_free(&job);
            continue;
        }

        // DEBUG: Display the user's command
        fprintf(stdout, "%s\n", job->cmds[0]->command);

        pid = fork();
        if (pid == 0) {
            execvp(job->cmds[0]->command, job->cmds[0]->args);
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

        fprintf(stderr, "+ completed '%s %s' [%d]\n",
                job->cmds[0]->command, (job->cmds[0]->args)[1], retval);

    } while(1);

    return EXIT_SUCCESS;
}