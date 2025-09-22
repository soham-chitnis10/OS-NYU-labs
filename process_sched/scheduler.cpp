#include "include/scheduler.h"
#include <iostream>
#include <algorithm>

// FCFS
FCFS::FCFS() { quantum = 10000; }
FCFS::FCFS(int q) { quantum = q; }
Process* FCFS::get_next_process() {
    if (runqueue.empty()) return nullptr;
    Process* proc = runqueue.front();
    runqueue.pop_front();
    return proc;
}
void FCFS::add_process(Process* proc) { runqueue.push_back(proc); }
void FCFS::display() {
    for (auto it : runqueue) printf("%d:%d ", it->id, it->state);
    std::cout << "\n";
}
bool FCFS::test_preempt(Process*) { return false; }

// LCFS
LCFS::LCFS() { quantum = 10000; }
Process* LCFS::get_next_process() {
    if (runqueue.empty()) return nullptr;
    Process* proc = runqueue.back();
    runqueue.pop_back();
    return proc;
}
void LCFS::add_process(Process* proc) { runqueue.push_back(proc); }
void LCFS::display() {
    for (auto it : runqueue) printf("%d:%d ", it->id, it->state);
    std::cout << "\n";
}
bool LCFS::test_preempt(Process*) { return false; }

// SRTF
SRTF::SRTF() { quantum = 10000; }
Process* SRTF::get_next_process() {
    if (runqueue.empty()) return nullptr;
    Process* proc = runqueue.front();
    runqueue.pop_front();
    return proc;
}
std::list<Process*>::iterator SRTF::find_process_it(int rem) {
    auto itr = runqueue.begin();
    for (; itr != runqueue.end(); ++itr) {
        int it_rem_time = (*itr)->total_cpu_time - (*itr)->cpu_time;
        if (it_rem_time > rem) return itr;
    }
    return runqueue.end();
}
void SRTF::add_process(Process* proc) {
    int rem = proc->total_cpu_time - proc->cpu_time;
    auto it = find_process_it(rem);
    if (it != runqueue.end())
        runqueue.insert(it, proc);
    else
        runqueue.push_back(proc);
}
void SRTF::display() {
    for (auto it : runqueue) printf("%d:%d ", it->id, it->state);
    std::cout << "\n";
}
bool SRTF::test_preempt(Process*) { return false; }

// PRIO
PRIO::PRIO() {}
PRIO::PRIO(int q, int mprios) {
    quantum = q;
    max_prios = mprios;
    activequeue_ptr = new std::vector<std::deque<Process*>>(max_prios);
    expiredqueue_ptr = new std::vector<std::deque<Process*>>(max_prios);
}
bool PRIO::check_empty(std::vector<std::deque<Process*>>* ptr) {
    for (int i = max_prios - 1; i >= 0; --i) {
        if (!ptr->at(i).empty()) return false;
    }
    return true;
}
Process* PRIO::get_next_process() {
    Process* proc = nullptr;
    if (check_empty(activequeue_ptr) && check_empty(expiredqueue_ptr)) return nullptr;
    if (!check_empty(activequeue_ptr)) {
        for (int i = max_prios - 1; i >= 0; --i) {
            if (!activequeue_ptr->at(i).empty()) {
                proc = activequeue_ptr->at(i).front();
                activequeue_ptr->at(i).pop_front();
                break;
            }
        }
    } else {
        std::vector<std::deque<Process*>>* swap = activequeue_ptr;
        activequeue_ptr = expiredqueue_ptr;
        expiredqueue_ptr = swap;
        return get_next_process();
    }
    return proc;
}
void PRIO::add_process(Process* proc) {
    if (proc->dynamic_prio < 0) {
        proc->dynamic_prio = proc->static_prio - 1;
        expiredqueue_ptr->at(proc->dynamic_prio).push_back(proc);
    } else {
        activequeue_ptr->at(proc->dynamic_prio).push_back(proc);
    }
}
void PRIO::display() {
    std::cout << "ActiveQ:\n";
    for (int i = max_prios - 1; i >= 0; --i) {
        std::cout << "Priority: " << i << std::endl;
        for (auto it : activequeue_ptr->at(i)) printf("%d:%d ", it->id, it->state);
        std::cout << "\n";
    }
    std::cout << "ExpiredQ:\n";
    for (int i = max_prios - 1; i >= 0; --i) {
        std::cout << "Priority: " << i;
        for (auto it : expiredqueue_ptr->at(i)) printf("%d:%d ", it->id, it->state);
        std::cout << "\n";
    }
}
bool PRIO::test_preempt(Process*) { return false; }

// EPRIO
EPRIO::EPRIO(int q, int mprios) {
    quantum = q;
    max_prios = mprios;
    activequeue_ptr = new std::vector<std::deque<Process*>>(max_prios);
    expiredqueue_ptr = new std::vector<std::deque<Process*>>(max_prios);
}
bool EPRIO::test_preempt(Process* proc) {
    extern Process* CURRENT_RUNNING_PROCESS;
    extern int CURRENT_TIME;
    extern std::list<Event*> event_queue;
    if (CURRENT_RUNNING_PROCESS == nullptr) return false;
    int next_event_time_current = -1;
    for (auto itr = event_queue.begin(); itr != event_queue.end(); ++itr) {
        if ((*itr)->evtProcess->id == CURRENT_RUNNING_PROCESS->id) {
            next_event_time_current = (*itr)->evtTimeStamp;
            break;
        }
    }
    return (proc->dynamic_prio > CURRENT_RUNNING_PROCESS->dynamic_prio) && (next_event_time_current > CURRENT_TIME);
}
