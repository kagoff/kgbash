#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "definitions.h"
#include "types.h"

#include "input.h"

/*
 * Fills a string to contain only the null-terminated user input without
 * a trailing endline character.
 * 
 * Returns:
 *  - False if input is empty
 *  - True otherwise
 */
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