
#include <stdio.h>
#include <string.h>
#include "kgbash.h"

/*
 * Returns a string that contains only the null-terminated user input without
 * a trailing endline character.
 */
 void parse_raw_input(char * string, size_t string_size) {
    int string_idx = 0;

    // Collect user input
    do {
        string[string_idx] = getchar();
    } while(string[string_idx++] != '\n' &&
            string_idx < string_size);

    // Clear input buffer if needed
    if(string[string_idx-1] != '\n') {
        while(getchar() != '\n');
        string[string_idx] = '\0';
    } else {
        string[string_idx-1] = '\0';
    }
}

int main() {

    char raw_input[MAX_INPUT_LENGTH + 1];

    do {
        // Reset user input
        memset(raw_input, 0, MAX_INPUT_LENGTH + 1);

        // Prompt user for input
        printf("kgbash: ");
        parse_raw_input(raw_input, MAX_INPUT_LENGTH + 1);

        // Display the user's command
        printf("%s\n", raw_input);
    } while(strncmp(raw_input, QUIT_STRING, sizeof(QUIT_STRING)-1));

    return 0;
}