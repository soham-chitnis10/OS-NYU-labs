#pragma once
#include <cstdint>
#include <vector>
#include "frame.h"

extern int num_frames;
extern bool O_flag, F_flag, P_flag, S_flag, x_flag, y_flag, f_flag, a_flag;
extern unsigned long long total_cost;
extern uint64_t total_inst, ctx_switches, process_exits;

int myrandom();
void print_frame_table();
void print_page_table(int pid);
void print_all_page_tables(int num_process);
