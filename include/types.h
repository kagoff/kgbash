#ifndef TYPES_H
#define TYPES_H

typedef enum kgbash_result {
    KGBASH_RET_SUCCESS = 0,
    KGBASH_RET_FAIL,
    KGBASH_RET_NULL_PARAM,
    KGBASH_RET_EMPTY_INPUT,
    KGBASH_RET_MISLOCATED_INPUT_REDIRECT,
    KGBASH_RET_MISLOCATED_OUTPUT_REDIRECT,
    KGBASH_RET_MISLOCATED_PIPE,
    KGBASH_RET_MISLOCATED_SLEEP
} kgbash_result_e;

typedef enum kgbash_item_type {
    KGBASH_ITEM_TYPE_EMPTY = 0,
    KGBASH_ITEM_TYPE_CMD_OR_ARG,
    KGBASH_ITEM_TYPE_INPUT_REDIRECT,
    KGBASH_ITEM_TYPE_OUTPUT_REDIRECT,
    KGBASH_ITEM_TYPE_PIPE,
    KGBASH_ITEM_TYPE_SLEEP,
    KGBASH_ITEM_TYPE_ERROR
} kgbash_item_type_e;

typedef struct cmd {
    char* args[ARG_ARRAY_LEN];
    size_t argc;
    int retval;
    pid_t pid;
} cmd_s;

typedef struct job {
    // TODO: dynamically allocate all of these to save memory
    cmd_s* cmds[INPUT_ARRAY_LEN];

    char raw_input[INPUT_ARRAY_LEN];
    char file[INPUT_ARRAY_LEN];

    uint8_t num_cmds;
    uint8_t pipes;

    bool redirect_out;
    bool redirect_in;
    bool sleep;
} job_s;

#endif //TYPES_H