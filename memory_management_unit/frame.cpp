#include "include/frame.h"

void frame_init(frame_t* frame) {
    frame->pid = -1;
    frame->vpage = -1;
    frame->age = 0;
    frame->last_time_used = 0;
}
