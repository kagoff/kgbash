#ifndef INPUT_H
#define INPUT_H

/*
 * Fills a string to contain only the null-terminated user input without
 * a trailing endline character.
 * 
 * Returns:
 *  - False if input is empty
 *  - True otherwise
 */
 bool input_parse_input(char * string, size_t string_size);

#endif //INPUT_H