#include "ioreq.h"

IOReq::IOReq(int ID, int at, int tn) : id(ID), arr_time(at), track_num(tn) {}

std::vector<IOReq*> IOs;
int io_add_ptr = 0;
IOReq* active = nullptr;
