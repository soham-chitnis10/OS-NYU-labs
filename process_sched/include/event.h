#ifndef EVENT_H
#define EVENT_H

#include "process.h"
#include <list>

class Event {
public:
    Process* evtProcess;
    int evtTimeStamp;
    int transition;
    Event(Process* proc, int time, int t);
};

extern std::list<Event*> event_queue;
Event* get_event();
void add_event(Process* proc, int time, int transition);
void display_eventQ();
int get_next_event_time();
using eventqit = std::list<Event*>::iterator;
eventqit find_it(int time);

#endif // EVENT_H
