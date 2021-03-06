
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

static void
check_running_jobs(void) {
    job_s* running_job;
    int running_job_retval;
    int total_jobs = queue_count(jobs);
    for(int num_jobs = 0; num_jobs < total_jobs; num_jobs++) {
        queue_dequeue(jobs, (void**)&running_job);

        // See if every command is complete in the job
        bool still_running = false;
        for(uint16_t pipes = (running_job->pipes + 1); pipes > 0; pipes--) {
            uint16_t cmd_idx = pipes - 1;
            if(!waitpid(running_job->cmds[cmd_idx]->pid,
                        &running_job->cmds[cmd_idx]->retval,
                        WNOHANG)) {
                still_running = true;
            }
        }
        // Output and free job if done, else throw back in the queue to check later
        if(!still_running) {
            output_completion_ret(running_job, KGBASH_RET_SUCCESS);
            job_free(&running_job);
        } else {
            queue_enqueue(jobs, (void*)running_job);
        }
        
    }
}

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
            check_running_jobs();
            continue;
        }

        // Create a job of commands from the user input
        job = job_create();
        ret = job_fill_from_input(job, input);
        if(ret != KGBASH_RET_SUCCESS) {
            output_completion_ret(job, ret);
            job_free(&job);
            check_running_jobs();
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
                check_running_jobs();
                continue;
            }
        }

        // Execute the job
        ret = job_run(job);

        // Free the job if not in the running queue
        if(!job->sleep || (job->sleep && ret != KGBASH_RET_SUCCESS)) {
            output_completion_ret(job, ret);
            job_free(&job);
        // Otherwise, enqueue the job if it was slept
        } else if(job->sleep) {
            queue_enqueue(jobs, (void*)job);
        }
        
        check_running_jobs();

    } while(1);

    return EXIT_SUCCESS;
}