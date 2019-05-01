#ifndef TYPES_H
#define TYPES_H

typedef enum kgbash_result {
    KGBASH_RET_SUCCESS = 0,
    KGBASH_RET_FAIL,
    KGBASH_RET_NULL_PARAM,
    KGBASH_RET_EMPTY_INPUT,
    KGBASH_RET_MISPLACED_INPUT_REDIRECT,
    KGBASH_RET_MISPLACED_OUTPUT_REDIRECT,
    KGBASH_RET_MISPLACED_PIPE
} kgbash_result_e;

typedef struct cmd {
    char* args[ARG_ARRAY_LEN];
    size_t argc;
} cmd_s;

typedef struct job {
    cmd_s* cmds[INPUT_ARRAY_LEN];
    int retvals[INPUT_ARRAY_LEN];
    char file[INPUT_ARRAY_LEN];
    bool *pipe_to_next;
    bool redirect_out;
    bool redirect_in;
    bool sleep;
} job_s;

#endif //TYPES_H