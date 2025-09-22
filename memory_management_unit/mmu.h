#pragma once
#include "include/process.h"
#include "include/frame.h"
#include "include/pager.h"
#include "include/utils.h"
#include <deque>
#include <vector>

extern frame_t frame_table[];
extern std::deque<frame_t*> free_frames;
extern std::vector<Process*> processes;
extern Process* current_process;

frame_t* allocate_frame_from_free_list();
frame_t* get_frame();
VMA* get_vma(std::vector<VMA*> vmas, uint32_t vpage);
