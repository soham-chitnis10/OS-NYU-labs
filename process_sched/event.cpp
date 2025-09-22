#include "include/event.h"
#include <cstdio>

std::list<Event*> event_queue;

Event::Event(Process* proc, int time, int t)
    : evtProcess(proc), evtTimeStamp(time), transition(t) {}

Event* get_event() {
    if (event_queue.empty()) return nullptr;
    Event* evt = event_queue.front();
    event_queue.pop_front();
    return evt;
}

eventqit find_it(int time) {
    eventqit itr = event_queue.begin();
    for (; itr != event_queue.end(); ++itr) {
        if ((*itr)->evtTimeStamp > time) return itr;
    }
    return event_queue.end();
}

void add_event(Process* proc, int time, int transition) {
    eventqit itr = find_it(time);
    if (itr != event_queue.end())
        event_queue.insert(itr, new Event(proc, time, transition));
    else
        event_queue.push_back(new Event(proc, time, transition));
}

void display_eventQ() {
    for (eventqit itr = event_queue.begin(); itr != event_queue.end(); ++itr) {
        printf("%d:%d:%d ", (*itr)->evtTimeStamp, (*itr)->evtProcess->id, (*itr)->evtProcess->state);
    }
}

int get_next_event_time() {
    if (event_queue.empty()) return -1;
    return event_queue.front()->evtTimeStamp;
}
