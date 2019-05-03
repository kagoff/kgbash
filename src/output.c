#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>

#include "definitions.h"
#include "types.h"

#include "output.h"

static void
output_completion(job_s* job) {
    if(!job || !(job->cmds[0])) {
        return;
    }

    fprintf(stderr, "+ completed '%s' ", job->raw_input);
    for(uint8_t cmd_idx; cmd_idx < job->num_cmds; cmd_idx++) {
        fprintf(stderr, "[%d]", WEXITSTATUS(job->cmds[cmd_idx]->retval));
    }

    fprintf(stderr, "\n");
}

void
output_completion_ret(job_s *job, kgbash_result_e ret) {
    switch(ret) {
        case KGBASH_RET_SUCCESS:
            output_completion(job);
            break;
        case KGBASH_RET_MISPLACED_INPUT_REDIRECT:
            // output_misplaced_input_redirect();
            break;
        case KGBASH_RET_MISPLACED_OUTPUT_REDIRECT:
            // output_misplaced_output_redirect();
            break;
        case KGBASH_RET_MISPLACED_PIPE:
            // output_misplaced_pipe();
            break;
        default:
            break;
    }
}