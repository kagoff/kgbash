
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "definitions.h"
#include "types.h"
#include "queue.h"
#include "input.h"
#include "output.h"
#include "job.h"

#include "kgbash.h"

static queue_t jobs = NULL;
// TODO: have job queue here

int main() {

    char input[INPUT_ARRAY_LEN];
    job_s *job;
    kgbash_result_e ret;

    // Create the running job queue
    jobs = queue_create();

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
        ret = job_fill_from_input(job, input);
        if(ret != KGBASH_RET_SUCCESS) {
            output_completion_ret(job, ret);
            job_free(&job);
            continue;
        }

        // Check for exit condition
        if(job_is_exit_string(job)) {
            if(!queue_count(jobs)) {
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
        ret = job_run(job);

        // Output result of current job
        output_completion_ret(job, ret);

        // Free the job if not in the running queue
        if(!job->sleep || (job->sleep && ret != KGBASH_RET_SUCCESS)) {
            job_free(&job);
        // Otherwise, enqueue the job if it was slept
        } else if(job->sleep) {
            queue_enqueue(jobs, (void*)job);
        }
        
        job_s* running_job;
        int running_job_retval;
        int total_jobs = queue_count(jobs);
        for(int num_jobs = 0; num_jobs < total_jobs; num_jobs++) {
            queue_dequeue(jobs, (void**)&running_job);

            // Check if the current job is done, and if so, free and output
            if(waitpid(running_job->pid, &running_job_retval, WNOHANG)) {
                output_completion_ret(job, ret);
                job_free(&job);
            } 
            // Throw the job back in the queue if not ready yet
            else {
                queue_enqueue(jobs, (void*)running_job);
            }
        }

    } while(1);

    return EXIT_SUCCESS;
}