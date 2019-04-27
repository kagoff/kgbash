#ifndef CMD_H
#define CMD_H

typedef struct cmd {
    char command[INPUT_ARRAY_LEN];
    char* args[ARG_ARRAY_LEN];
} cmd_s;

#endif //CMD_H