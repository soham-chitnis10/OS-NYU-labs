#pragma once
#include <cstdint>

struct frame_t {
    uint32_t pid;
    uint32_t vpage;
    uint32_t age;
    uint32_t last_time_used;
};

void frame_init(frame_t* frame);
