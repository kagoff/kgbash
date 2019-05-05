#ifndef JOBS_H
#define JOBS_H

job_s *job_create(void);

void job_free(job_s **job);

bool job_is_exit_string(const job_s *job);

kgbash_result_e job_fill_from_input (job_s* job, const char* string);

kgbash_result_e job_run (job_s *job);

#endif //CMDS_H