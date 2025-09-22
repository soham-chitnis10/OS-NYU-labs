#ifndef IOREQ_H
#define IOREQ_H

#include <vector>

class IOReq {
public:
    int id;
    int arr_time;
    int start_time;
    int end_time;
    int track_num;
    IOReq(int ID, int at, int tn);
};

extern std::vector<IOReq*> IOs;
extern int io_add_ptr;
extern IOReq* active;

#endif // IOREQ_H
