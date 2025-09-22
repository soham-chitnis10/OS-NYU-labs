#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include <deque>
#include <vector>
#include <list>

class Scheduler {
public:
    int quantum;
    virtual Process* get_next_process() = 0;
    virtual void add_process(Process*) = 0;
    virtual void display() = 0;
    virtual bool test_preempt(Process*) = 0;
    virtual ~Scheduler() {}
};

class FCFS : public Scheduler {
    std::deque<Process*> runqueue;
public:
    FCFS();
    FCFS(int q);
    Process* get_next_process() override;
    void add_process(Process* proc) override;
    void display() override;
    bool test_preempt(Process* proc) override;
};

class LCFS : public Scheduler {
    std::deque<Process*> runqueue;
public:
    LCFS();
    Process* get_next_process() override;
    void add_process(Process* proc) override;
    void display() override;
    bool test_preempt(Process* proc) override;
};

class SRTF : public Scheduler {
    std::list<Process*> runqueue;
public:
    SRTF();
    Process* get_next_process() override;
    void add_process(Process* proc) override;
    void display() override;
    bool test_preempt(Process* proc) override;
private:
    std::list<Process*>::iterator find_process_it(int rem);
};

class PRIO : public Scheduler {
protected:
    std::vector<std::deque<Process*>>* activequeue_ptr;
    std::vector<std::deque<Process*>>* expiredqueue_ptr;
    int max_prios;
public:
    PRIO();
    PRIO(int q, int mprios);
    Process* get_next_process() override;
    void add_process(Process* proc) override;
    void display() override;
    bool test_preempt(Process* proc) override;
    bool check_empty(std::vector<std::deque<Process*>>* ptr);
};

class EPRIO : public PRIO {
public:
    EPRIO(int q, int mprios);
    bool test_preempt(Process* proc) override;
};

#endif // SCHEDULER_H
