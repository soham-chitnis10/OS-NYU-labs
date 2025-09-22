#ifndef IOSCHED_H
#define IOSCHED_H

#include "ioreq.h"
#include <queue>
#include <vector>

class IOSchedBase {
public:
    virtual IOReq* strategy() = 0;
    virtual void add_req(IOReq* io_req) = 0;
    virtual void completed_req() = 0;
    virtual void print_queue() = 0;
    virtual bool all_processed() = 0;
    virtual ~IOSchedBase() {}
};

class FIFO : public IOSchedBase {
    std::queue<IOReq*> io_queue;
public:
    IOReq* strategy() override;
    void add_req(IOReq* io_req) override;
    void completed_req() override;
    void print_queue() override;
    bool all_processed() override;
};

class SSTF : public IOSchedBase {
protected:
    std::vector<IOReq*> io_queue;
public:
    IOReq* strategy() override;
    void add_req(IOReq* req) override;
    void completed_req() override;
    void print_queue() override;
    bool all_processed() override;
};

class LOOK : public SSTF {
protected:
    int direction = 1;
public:
    IOReq* strategy() override;
    void print_queue() override;
};

class CLOOK : public LOOK {
public:
    IOReq* strategy() override;
};

class FLOOK : public IOSchedBase {
    std::vector<IOReq*> *active_queue_ptr;
    std::vector<IOReq*> *add_queue_ptr;
    int direction = 1;
public:
    FLOOK();
    ~FLOOK();
    IOReq* strategy() override;
    void add_req(IOReq* req) override;
    void completed_req() override;
    void print_queue() override;
    bool all_processed() override;
};

#endif // IOSCHED_H
