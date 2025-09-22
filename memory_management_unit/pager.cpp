#include "include/pager.h"
#include "include/process.h"
#include <vector>
#include <cstring>
#include <deque>
extern frame_t frame_table[];
extern int num_frames;
extern std::vector<Process*> processes;
extern uint64_t total_inst;

FIFO::FIFO() { hand = 0; }
frame_t* FIFO::select_victim_frame() {
    frame_t* frame = &(frame_table[(hand++) % num_frames]);
    return frame;
}

Random::Random() { hand = 0; }
frame_t* Random::select_victim_frame() {
    hand = myrandom() % num_frames;
    frame_t* frame = &(frame_table[hand]);
    return frame;
}

Clock::Clock() { hand = 0; }
frame_t* Clock::select_victim_frame() {
    while (true) {
        frame_t* frame = &(frame_table[(hand++) % num_frames]);
        if (!processes[frame->pid]->page_table[frame->vpage].referenced) {
            return frame;
        } else {
            processes[frame->pid]->page_table[frame->vpage].referenced = 0;
        }
    }
}

NRU::NRU() { last_inst_reset = 0; hand = 0; }
frame_t* NRU::select_victim_frame() {
    frame_t* frame = nullptr;
    memset(class_frames, -1, sizeof(int) * 4);
    bool reset = ((total_inst - last_inst_reset + 1) >= 48);
    for (int i = 0; i < num_frames; i++) {
        frame_t* curr_frame = &(frame_table[(hand + i) % num_frames]);
        pte_t* pte = &(processes[curr_frame->pid]->page_table[curr_frame->vpage]);
        int class_id = pte->referenced * 2 + pte->modified;
        if (class_id == 0 && class_frames[0] == -1) {
            frame = curr_frame;
            hand = (hand + i + 1);
            break;
        } else if (class_frames[class_id] == -1) {
            class_frames[class_id] = curr_frame - frame_table;
        }
    }
    if (frame == nullptr) {
        for (int i = 0; i < 4; i++) {
            if (class_frames[i] != -1) {
                frame = &(frame_table[class_frames[i]]);
                hand = class_frames[i] + 1;
                break;
            }
        }
    }
    if (reset) {
        for (int i = 0; i < num_frames; i++) {
            pte_t* pte = &(processes[frame_table[i].pid]->page_table[frame_table[i].vpage]);
            pte->referenced = 0;
        }
        last_inst_reset = total_inst + 1;
    }
    return frame;
}

Aging::Aging() { hand = 0; }
frame_t* Aging::select_victim_frame() {
    frame_t* frame = &(frame_table[(hand) % num_frames]);
    for (int i = 0; i < num_frames; i++) {
        frame_t* curr_frame = &(frame_table[(hand + i) % num_frames]);
        curr_frame->age = curr_frame->age >> 1;
        pte_t* pte = &(processes[curr_frame->pid]->page_table[curr_frame->vpage]);
        if (pte->referenced) {
            curr_frame->age = curr_frame->age | 0x80000000;
        }
        pte->referenced = 0;
        if (curr_frame->age < frame->age) {
            frame = curr_frame;
        }
    }
    hand = frame - frame_table + 1;
    return frame;
}

WorkingSet::WorkingSet() { tau = 49; }
frame_t* WorkingSet::select_victim_frame() {
    uint32_t smallest_time = 0;
    frame_t* oldest_frame = nullptr;
    int start = hand;
    while (true) {
        frame_t* frame = &(frame_table[(hand++) % num_frames]);
        pte_t* pte = &(processes[frame->pid]->page_table[frame->vpage]);
        int age = (total_inst - frame->last_time_used);
        if (start % num_frames == frame - frame_table && (hand - 1) > start) {
            if (oldest_frame == nullptr)
                oldest_frame = frame;
            hand = oldest_frame - frame_table + 1;
            return oldest_frame;
        }
        if (pte->referenced) {
            pte->referenced = 0;
            frame->last_time_used = total_inst;
        } else if (age > tau) {
            return frame;
        } else if (age > smallest_time) {
            smallest_time = age;
            oldest_frame = frame;
        }
    }
}
