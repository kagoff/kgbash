#ifndef TYPES_H
#define TYPES_H

typedef struct cmd {
    char command[INPUT_ARRAY_LEN];
    char* args[ARG_ARRAY_LEN];
    size_t argc;
} cmd_s;

typedef struct job {
    cmd_s* cmds[INPUT_ARRAY_LEN];
    bool *pipe_to_next;
    bool redirect_out;
    bool redirect_in;
    bool sleep;
} job_s;

#endif //TYPES_H