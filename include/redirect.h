#ifndef REDIRECT_H
#define REDIRECT_H

kgbash_result_e redirect_file_out(const char* file, int* fd);

kgbash_result_e redirect_file_in(const char* file, int* fd);

void redirect_reset_file_descriptors(int stdin_fd, int stdout_fd);

#endif //REDIRECT_H