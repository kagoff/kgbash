#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "definitions.h"
#include "types.h"

#include "cmd.h"

cmd_s *cmd_create(void) {
    cmd_s *cmd = malloc(sizeof(cmd_s));

    cmd->argc = 0;
    cmd->retval = 0;
    for(int arg_idx = 0; arg_idx < ARG_ARRAY_LEN; arg_idx++) {
        cmd->args[arg_idx] = NULL;
    }

    return cmd;
}

void cmd_free(cmd_s **cmd_ptr) {
    if(!cmd_ptr || !(*cmd_ptr)) {
        return;
    }
    cmd_s *cmd = *cmd_ptr;

    uint32_t arg_idx = 0;
    while(arg_idx < cmd->argc) {
        free(cmd->args[arg_idx++]);
    }

    free(cmd);
    cmd = NULL;
}