#ifndef TYPES_H
#define TYPES_H

typedef struct cmd {
    char command[INPUT_ARRAY_LEN];
    char* args[ARG_ARRAY_LEN];
} cmd_s;

#endif //TYPES_H