
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "definitions.h"
#include "types.h"
#include "input.h"
#include "output.h"
#include "job.h"

#include "kgbash.h"

static bool active_jobs = false;
// TODO: have job queue here

int main() {

    char input[INPUT_ARRAY_LEN];
    job_s *job;

    do {
        // Reset user input
        memset(input, 0, INPUT_ARRAY_LEN*sizeof(char));

        // Prompt user for input
        fprintf(stdout, "kgbash$ ");

        // Gather user input data, and skip fork if empty
        if(!input_parse_input(input, INPUT_ARRAY_LEN)) {
            continue;
        }

        // Create a job of commands from the user input
        job = job_create();
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
        // TODO: make sure this can be piped and output redirected
        if(job_run_internal(job)) {
            output_completion(job);
            job_free(&job);
            continue;
        }

        // Execute the job
        if(job->sleep) {
            job_run_background(job);
            continue;
        } else {
            job_run(job);
        }

        // Output result and free the job to start over
        output_completion(job);
        job_free(&job);

    } while(1);

    return EXIT_SUCCESS;
}