
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
    kgbash_result_e ret;

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
        if(job_fill_from_input(job, input) != KGBASH_RET_SUCCESS) {
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

        // Execute the job
        ret = job_run(job, job->sleep);

        // Don't free memory if the job was slept
        // TODO: this should actually not be needed and can be taken care of
        //      within job_run call
        if(job->sleep) {
            continue;
        }

        // Output result and free the job to start over
        output_completion(job);
        job_free(&job);

        // TODO: check here for completed jobs

    } while(1);

    return EXIT_SUCCESS;
}