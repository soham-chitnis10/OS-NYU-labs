#include "include/process.h"

Process::Process(State s, int st_ts, int sprio, int dprio, int tot_cpu, int cb, int io, int ID, int AT, int FT, int IT, int CW, int cpu_time_init, int ccb)
    : state(s), state_ts(st_ts), static_prio(sprio), dynamic_prio(dprio), total_cpu_time(tot_cpu), CB(cb), IO(io), id(ID), at(AT), ft(FT), it(IT), cw(CW), cpu_time(cpu_time_init), current_cb(ccb) {}
