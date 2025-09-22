#ifndef PROCESS_H
#define PROCESS_H

#include <iostream>

enum State { STATE_CREATED, STATE_READY, STATE_RUNNING, STATE_BLOCKED, STATE_DONE };

class Process {
public:
    State state;
    int state_ts;
    int static_prio;
    int dynamic_prio;
    int total_cpu_time;
    int at;
    int CB;
    int IO;
    int id;
    int ft;
    int it;
    int cw;
    int cpu_time;
    int current_cb;
    Process(State s, int st_ts, int sprio, int dprio, int tot_cpu, int cb, int io, int ID, int AT, int FT, int IT, int CW, int cpu_time_init, int ccb);
};

#endif // PROCESS_H
