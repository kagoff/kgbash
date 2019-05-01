#ifndef JOBS_H
#define JOBS_H

job_s *job_create(void);

void job_free(job_s **job);

bool job_is_exit_string(const job_s *job);

bool job_fill_from_input (job_s* job, const char* string);

bool job_run_internal(job_s* job);

void job_run (job_s *job, bool sleep);

#endif //CMDS_H