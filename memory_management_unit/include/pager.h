#pragma once
#include "frame.h"
#include <deque>

class Pager {
public:
    int hand;
    virtual frame_t* select_victim_frame() = 0;
    virtual ~Pager() = default;
};

class FIFO : public Pager {
public:
    FIFO();
    frame_t* select_victim_frame() override;
};

class Random : public Pager {
public:
    Random();
    frame_t* select_victim_frame() override;
};

class Clock : public Pager {
public:
    Clock();
    frame_t* select_victim_frame() override;
};

class NRU : public Pager {
    int last_inst_reset;
    int class_frames[4];
public:
    NRU();
    frame_t* select_victim_frame() override;
};

class Aging : public Pager {
public:
    Aging();
    frame_t* select_victim_frame() override;
};

class WorkingSet : public Pager {
public:
    int tau;
    WorkingSet();
    frame_t* select_victim_frame() override;
};
