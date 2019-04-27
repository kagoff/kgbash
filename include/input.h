#ifndef INPUT_H
#define INPUT_H

#include "definitions.h"
#include "types.h"

#include "input.h"

bool input_is_exit_string(const char* cmd);

/*
 * Assumes
 */
bool input_separate_commands (cmd_s *cmd, const char* string, size_t n_args, size_t string_size);

/*
 * Fills a string to contain only the null-terminated user input without
 * a trailing endline character.
 * 
 * Returns:
 *  - False if input is empty
 *  - True otherwise
 */
 bool input_parse_raw_input(char * string, size_t string_size);

#endif //INPUT_H