#ifndef TYPES_H
#define TYPES_H

typedef struct cmd {
    char command[INPUT_ARRAY_LEN];
    char* args[ARG_ARRAY_LEN];
} cmd_s;

typedef struct job {
    cmd_s **cmds;
    bool *pipe_to_next;
    bool redirect_out;
    bool reirect_in;
    bool sleep;
} job_s;

#endif //TYPES_H