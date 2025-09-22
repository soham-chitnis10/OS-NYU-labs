#ifndef UTILS_H
#define UTILS_H

#include "process.h"
#include <map>

extern int CURRENT_TIME;
extern int IO_BUSY_TIME;
extern bool CALL_SCHEDULER;
extern Process* CURRENT_RUNNING_PROCESS;
extern long* randvals;
extern bool vflag, tflag, eflag, pflag, IO_USE;
extern int IO_started_at, IO_lasts_till, ofs;
extern std::map<int, Process*> proc_info;

int myrandom(int burst);
void display_summary();

#endif // UTILS_H
