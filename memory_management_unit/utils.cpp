#include "include/utils.h"
#include "include/process.h"
#include <cstdio>
#include <iostream>
#include <cstring>
#include <vector>
using namespace std;

extern frame_t frame_table[];
extern std::vector<Process*> processes;

void print_frame_table() {
    cout << "FT:";
    for (int i = 0; i < num_frames; i++) {
        if (frame_table[i].pid != -1) {
            printf(" %d:%d", frame_table[i].pid, frame_table[i].vpage);
        } else {
            printf(" *");
        }
    }
    cout << "\n";
}

void print_page_table(int pid) {
    printf("PT[%d]:", pid);
    for (int j = 0; j < 64; j++) {
        if (!processes[pid]->page_table[j].valid) {
            if (processes[pid]->page_table[j].pagedout) {
                printf(" #");
            } else {
                printf(" *");
            }
            continue;
        }
        printf(" %d:%c%c%c", j, (processes[pid]->page_table[j].referenced ? 'R' : '-'), (processes[pid]->page_table[j].modified ? 'M' : '-'), (processes[pid]->page_table[j].pagedout ? 'S' : '-'));
    }
    cout << "\n";
}

void print_all_page_tables(int num_process) {
    for (int i = 0; i < num_process; i++) {
        print_page_table(i);
    }
}
