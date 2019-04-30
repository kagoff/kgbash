#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "definitions.h"
#include "types.h"

#include "output.h"

void output_completion(job_s* job) {
    if(!job || !(job->cmds[0])) {
        return;
    }

    fprintf(stderr, "+ completed '%s", job->cmds[0]->args[0]);
    
    for(uint32_t arg_idx = 1; arg_idx < job->cmds[0]->argc; arg_idx++) {
        fprintf(stderr, " %s", job->cmds[0]->args[arg_idx]);
    }

    fprintf(stderr, "' [%d]\n", job->retvals[0]);
}