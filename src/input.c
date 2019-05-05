#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "definitions.h"
#include "types.h"

#include "input.h"

// Local character checking functions
static inline bool is_output_redirect(char token) {
    return token == '>' ? true : false;
}
static inline bool is_input_redirect(char token) {
    return token == '<' ? true : false;
}
static inline bool is_pipe(char token) {
    return token == '|' ? true : false;
}
static inline bool is_null(char token) {
    return token == '\0' ? true: false;
}
static inline bool is_white_space(char token) {
    if(token == ' ' || token == '\t') {
        return true;
    }
    return false;
}
static inline bool is_white_space_or_null(char token) {
    switch(token) {
        case ' ':
        case '\t':
        case '\0':
            return true;
        default:
            return false;
    }
}
static inline bool is_special_token(char token) {
    switch(token) {
        case '|':
        case '<':
        case '>':
        case '&':
            return true;
        default:
            return false;
    }
}
static inline bool is_white_space_or_null_or_special_token(char token) {
    if(is_white_space_or_null(token) || is_special_token(token)) {
        return true;
    }
    return false;
}
static inline void clear_leading_whitespace(const char* string, uint16_t *str_idx) {
    while(is_white_space(string[*str_idx])) {
        (*str_idx)++;
    }
}

kgbash_item_type_e
input_grab_next_item(const char* string, uint16_t *str_idx, char* item, uint16_t *item_idx) {
    // Prepare the string for input read
    if(!string || !str_idx || !item) {
        return KGBASH_ITEM_TYPE_ERROR;
    }
    clear_leading_whitespace(string, str_idx);

    // Return emtpy if nothing left
    if(is_null(string[*str_idx])) {
        return KGBASH_ITEM_TYPE_EMPTY;
    }

    // Fill the item until break point, and cap with null
    *item_idx = 0;
    while(!is_white_space_or_null_or_special_token(string[*str_idx]) &&
           *str_idx < MAX_INPUT_LENGTH && *item_idx < MAX_INPUT_LENGTH) {
        item[(*item_idx)++] = string[(*str_idx)++];
    }
    item[*item_idx] = '\0';

    // If only one character long, check for special token
    if(item[0] == '\0') {
        item[0] = string[(*str_idx)++];
        switch(item[0]) {
            case '|':
                return KGBASH_ITEM_TYPE_PIPE;
            case '<':
                return KGBASH_ITEM_TYPE_INPUT_REDIRECT;
            case '>':
                return KGBASH_ITEM_TYPE_OUTPUT_REDIRECT;
            case '&':
                return KGBASH_ITEM_TYPE_SLEEP;
            default: break;
        }
    }

    // At this pont, it has to be a command, arg, or file
    return KGBASH_ITEM_TYPE_CMD_OR_ARG;
}

bool input_parse_input(char * string, size_t string_size) {
    uint32_t string_idx = 0;

    // Collect user input
    do {
        string[string_idx] = getchar();
    } while(string[string_idx++] != '\n' &&
            string_idx < string_size);

    // Return false on empty user input
    if(string[0] == '\n') {
        return false;
    }

    // Clear input buffer if needed
    if(string[string_idx-1] != '\n') {
        while(getchar() != '\n');
        string[string_idx] = '\0';
    } else {
        string[string_idx-1] = '\0';
    }

    return true;
}