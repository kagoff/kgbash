#include <stdbool.h>
#include <string.h>

#include "definitions.h"
#include "types.h"

#include "cmds.h"

// TODO: write this
bool cmds_run_internal(const char* cmd) {
    if(!cmd) {
        return false;
    }
    if(!strncmp(cmd, PWD_STRING, sizeof(PWD_STRING))) {
        return true;
    }
    if(!strncmp(cmd, CD_STRING, sizeof(CD_STRING))) {
        return true;
    }
    return false;
}