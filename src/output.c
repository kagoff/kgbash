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
    for(uint16_t cmd_idx= 0; cmd_idx < job->num_cmds; cmd_idx++) {
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
        case KGBASH_RET_MISLOCATED_INPUT_REDIRECT:
            fprintf(stderr, "Error: mislocated input redirect\n");
            break;
        case KGBASH_RET_MISLOCATED_OUTPUT_REDIRECT:
            fprintf(stderr, "Error: mislocated output redirect\n");
            break;
        case KGBASH_RET_MISLOCATED_PIPE:
            fprintf(stderr, "Error: mislocated pipe\n");
            break;
        case KGBASH_RET_MISLOCATED_SLEEP:
            fprintf(stderr, "Error: mislocated sleep\n");
            break;
        default:
            break;
    }
}