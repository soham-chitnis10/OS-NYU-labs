#include "include/utils.h"
#include <cstdio>
#include <algorithm>

int CURRENT_TIME = 0;
int IO_BUSY_TIME = 0;
bool CALL_SCHEDULER = false;
Process* CURRENT_RUNNING_PROCESS = nullptr;
long* randvals;
bool vflag = false, tflag = false, eflag = false, pflag = false, IO_USE = false;
int IO_started_at = -1, IO_lasts_till = -1, ofs = 0;
std::map<int, Process*> proc_info;

int myrandom(int burst) {
    return 1 + (randvals[ofs++] % burst);
}

void display_summary() {
    int cpu_busy_time = 0;
    int num_processes = proc_info.size();
    int finish_time = CURRENT_TIME;
    int total_tt = 0;
    int total_cw = 0;
    for (auto it : proc_info) {
        printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n", it.first, it.second->at, it.second->total_cpu_time, it.second->CB, it.second->IO, it.second->static_prio, it.second->ft, (it.second->ft - it.second->at), it.second->it, it.second->cw);
        cpu_busy_time += it.second->total_cpu_time;
        total_tt += (it.second->ft - it.second->at);
        total_cw += it.second->cw;
    }
    double cpu_util = 100.0 * (cpu_busy_time / (double)finish_time);
    double io_util = 100.0 * (IO_BUSY_TIME / (double)finish_time);
    double avg_tt = (total_tt / (double)num_processes);
    double avg_cw = (total_cw / (double)num_processes);
    double throughput = 100.0 * (num_processes / (double)finish_time);
    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n", finish_time, cpu_util, io_util, avg_tt, avg_cw, throughput);
}
