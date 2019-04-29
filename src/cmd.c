#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "definitions.h"
#include "types.h"

#include "cmd.h"

cmd_s *cmd_create(void) {
    cmd_s *cmd = malloc(sizeof(cmd_s));

    memset(cmd->command, 0, INPUT_ARRAY_LEN*sizeof(char));
    cmd->argc = 0;
    for(int arg_idx = 0; arg_idx < ARG_ARRAY_LEN; arg_idx++) {
        cmd->args[arg_idx] = NULL;
    }

    return cmd;
}

void cmd_free(cmd_s *cmd) {
    if(!cmd) {
        return;
    }
    uint32_t arg_idx = 0;
    while(arg_idx < cmd->argc) {
        free((cmd->args)[arg_idx++]);
    }
    // TODO: figure out how to do this properly
    // free(cmd);
}