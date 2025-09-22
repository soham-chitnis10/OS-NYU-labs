#include <stdio.h>
#include <iostream>
#include <string.h>
#include <strings.h>
#include <bits/stdc++.h>
using namespace std;

bool v_flag = false;
bool f_flag = false;
bool q_flag = false;

int total_time = 0;
int total_movement =0;
int curr_time = 1;
int curr_track = 0;
int total_ios = 0;
bool check_completed =true;
class IOReq{
    public:
    int id;
    int arr_time;
    int start_time;
    int end_time;
    int track_num;

    IOReq(int ID, int at, int tn)
    {
        id = ID;
        arr_time = at;
        track_num = tn;
    }
};
vector<IOReq*> IOs;
int io_add_ptr = 0;
IOReq* active = nullptr;

class IOSchedBase
{
public:
    virtual IOReq* strategy() = 0;
    virtual void add_req(IOReq* io_req)=0;
    virtual void completed_req() = 0;
    virtual void print_queue() = 0;
    virtual bool all_processed() = 0;
};

class FIFO : public IOSchedBase
{
    queue<IOReq*> io_queue;
    IOReq* strategy()
    {
        if (io_queue.empty())
            return nullptr;
        return io_queue.front();
    }
    void add_req(IOReq* io_req)
    {
        io_queue.push(io_req);
    }
    void completed_req()
    {
        io_queue.pop();
    }
    void print_queue()
    {
        queue<IOReq*> tempQ = io_queue;
        printf("\tGet: (");
        while(!tempQ.empty())
        {
            IOReq* req = tempQ.front();
            printf("%d:%d ",req->id,abs(req->track_num-curr_track));
            tempQ.pop();
        }
        printf(") --> %d\n",io_queue.front()->id);
    }
    bool all_processed()
    {
        return io_queue.empty();
    }
};
bool compare_seek_time(IOReq* a, IOReq* b)
{
    return abs(a->track_num-curr_track) < abs(b->track_num-curr_track);
}
class SSTF: public IOSchedBase{
    protected:
    vector<IOReq*> io_queue;
    IOReq* strategy()
    {
        if (io_queue.empty())
        {
            return nullptr;
        }
        IOReq * selected = nullptr;
        int min_dist = INT_MAX;
        for(auto req: io_queue)
        {
            if(abs(req->track_num-curr_track) < min_dist)
            {
                min_dist = abs(req->track_num-curr_track);
                selected = req;
            }
        }
        return selected;
    }
    void add_req(IOReq* req)
    {
        io_queue.push_back(req);
    }
    void completed_req()
    {
        auto it = find(io_queue.begin(), io_queue.end(), active);
        io_queue.erase(it);
    }
    void print_queue()
    {
        printf("\tGet: (");
        for(auto req: io_queue)
        {
            printf("%d:%d ",req->id,abs(req->track_num-curr_track));
        }
        printf(") --> %d\n",active->id);
    }
    bool all_processed()
    {
        return io_queue.empty();
    }
};

class LOOK: public SSTF{
    protected:
    int direction=1;
    IOReq* strategy()
    {
        if(io_queue.empty())
        {
            return nullptr;
        }
        IOReq * selected_up = nullptr;
        IOReq * selected_down = nullptr;
        int min_dist_up = INT_MAX;
        int min_dist_down = INT_MAX;
        for(auto req: io_queue)
        {
            if(req->track_num == curr_track)
            {
                return req;
            }
            else if(req->track_num> curr_track)
            {
                if((req->track_num - curr_track) < min_dist_up)
                {
                    min_dist_up = (req->track_num - curr_track);
                    selected_up = req;
                }
            }
            else
            {
                if((curr_track - req->track_num) < min_dist_down)
                {
                    min_dist_down = (curr_track - req->track_num);
                    selected_down = req;
                }
            }
        }
        if(direction == 1)
        {
            if(min_dist_up == INT_MAX)
            {
                direction=-1;
                return selected_down;
            }
            return selected_up;
        }
        else
        {
            if(min_dist_down == INT_MAX)
            {
                direction=1;
                return selected_up;
            }
            return selected_down;
        }
    }
    void print_queue()
    {
        printf("\tGet: (");
        for(auto req: io_queue)
        {
            printf("%d:%d ",req->id,(req->track_num-curr_track));
        }
        printf(") --> %d\n",active->id);
    }
};

class CLOOK: public LOOK{
    IOReq* strategy()
    {
        if(io_queue.empty())
        {
            return nullptr;
        }
        IOReq * selected_up = nullptr;
        IOReq * selected_down = nullptr;
        int min_dist_up = INT_MAX;
        int max_dist_down = INT_MIN;
        for(auto req: io_queue)
        {
            if(req->track_num == curr_track)
            {
                return req;
            }
            else if(req->track_num> curr_track)
            {
                if((req->track_num - curr_track) < min_dist_up)
                {
                    min_dist_up = (req->track_num - curr_track);
                    selected_up = req;
                }
            }
            else
            {
                if((curr_track - req->track_num) > max_dist_down)
                {
                    max_dist_down = (curr_track - req->track_num);
                    selected_down = req;
                }
            }
        }
        if(min_dist_up == INT_MAX)
        {
            return selected_down;
        }
        return selected_up;

    }
};

class FLOOK: public IOSchedBase{
        vector<IOReq*> * active_queue_ptr = new vector<IOReq*>();
        vector<IOReq*> * add_queue_ptr = new vector<IOReq*>();
        int direction=1;
    IOReq* strategy()
    {
        if(active_queue_ptr->empty() && add_queue_ptr->empty())
        {
            return nullptr;
        }
        if(active_queue_ptr->empty())
        {
            vector<IOReq*> * tmp = active_queue_ptr;
            active_queue_ptr = add_queue_ptr;
            add_queue_ptr = tmp;
            direction=1;
            return strategy();
        }
        IOReq * selected_up = nullptr;
        IOReq * selected_down = nullptr;
        int min_dist_up = INT_MAX;
        int min_dist_down = INT_MAX;
        for(auto req: *(active_queue_ptr))
        {
            if(req->track_num == curr_track)
            {
                return req;
            }
            else if(req->track_num> curr_track)
            {
                if((req->track_num - curr_track) < min_dist_up)
                {
                    min_dist_up = (req->track_num - curr_track);
                    selected_up = req;
                }
            }
            else
            {
                if((curr_track - req->track_num) < min_dist_down)
                {
                    min_dist_down = (curr_track - req->track_num);
                    selected_down = req;
                }
            }
        }
        if(direction == 1)
        {
            if(min_dist_up == INT_MAX)
            {
                direction=-1;
                return selected_down;
            }
            return selected_up;
        }
        else
        {
            if(min_dist_down == INT_MAX)
            {
                direction=1;
                return selected_up;
            }
            return selected_down;
        }
    }
    void add_req(IOReq* req)
    {
        add_queue_ptr->push_back(req);
    }
    void completed_req()
    {
        auto it = find(active_queue_ptr->begin(), active_queue_ptr->end(), active);
        active_queue_ptr->erase(it);
    }
    void print_queue()
    {
        printf("\tGet: (");
        for(auto req: *(active_queue_ptr))
        {
            printf("%d:%d ",req->id,(req->track_num-curr_track));
        }
        printf(") --> %d\n",active->id);
    }
    bool all_processed()
    {
        return active_queue_ptr->empty() && add_queue_ptr->empty();
    }
};

int main(int argc, char * argv[])
{
    char algo = 'N';
    int op;
    while( (op = getopt(argc, argv, "s:vqf")) != -1)
    {
        switch(op)
        {
            case 's':
                sscanf(optarg, "%c", &algo);
                break;
            case 'v':
                v_flag = true;
                break;
            case 'q':
                q_flag = true;
                break;
            case 'f':
                f_flag = true;
                break;
            default:
                abort();
        }
    }
    IOSchedBase * io_sched;
    switch(algo)
    {
        case 'N':
            io_sched = new FIFO();
            break;
        case 'S':
            io_sched = new SSTF();
            break;
        case 'L':
            io_sched = new LOOK();
            break;
        case 'C':
            io_sched = new CLOOK();
            break;
        case 'F':
            io_sched = new FLOOK();
            break;
        default:
            break;
    }
    char * inputfile = argv[argc-1];
    FILE* fp = fopen(inputfile, "r");
    char * str = NULL;
    size_t n = 0;
    while (getline(&str,&n,fp)!=-1)
    {
        if(str[0] == '#')
        {
            continue;
        }
        int arr_time, track_num;
        sscanf(str, "%d %d", &arr_time, &track_num);
        IOReq * req = new IOReq(total_ios++, arr_time, track_num);
        IOs.push_back(req);
    }
    if(v_flag)
    {
        printf("TRACE\n");
    }
    while(true)
    {
        if(io_add_ptr < IOs.size() && curr_time==IOs[io_add_ptr]->arr_time)
        {
            io_sched->add_req(IOs[io_add_ptr]);
            if(v_flag)
            {
                printf("%d: %5d add %d\n", curr_time, IOs[io_add_ptr]->id,  IOs[io_add_ptr]->track_num);
            }
            io_add_ptr++;
        }
        while(check_completed)
        {
            if(active)
            {
                if(active->track_num==curr_track)
                {
                    active->end_time = curr_time;
                    if(v_flag)
                    {
                        printf("%d: %5d finish %d\n", curr_time, active->id, active->end_time - active->arr_time);
                    }
                    io_sched->completed_req();
                    active = nullptr;
                }
                else
                {
                    check_completed=false;
                }
            }
            if(active==nullptr)
            {
                active = io_sched->strategy();
                if(active==nullptr)
                {
                    break;
                }
                active->start_time = curr_time;
                if(q_flag)
                {
                    io_sched->print_queue();
                }
                if(v_flag)
                {
                    printf("%d: %5d issue %d %d\n", curr_time, active->id,  active->track_num, curr_track);
                }
            }
        }
        if(active)
        {
            if(curr_track  < active->track_num)
            curr_track++;
            else
            curr_track--;

            total_movement++;
        }
        if(io_sched->all_processed() && io_add_ptr == total_ios)
        {
            break;
        }
        curr_time++;
        check_completed =true;
    }
    int io_busy_time =0;
    int tot_turnaround_time= 0;
    int tot_wait_time = 0;
    int maxwaittime = INT_MIN;
    for(auto io: IOs)
    {
        printf("%5d: %5d %5d %5d\n",io->id,io->arr_time,io->start_time,io->end_time);
        io_busy_time+= (io->end_time - io->start_time);
        tot_turnaround_time+= (io->end_time - io->arr_time);
        tot_wait_time += (io->start_time-io->arr_time);
        maxwaittime = max(maxwaittime, (io->start_time-io->arr_time));
    }
    double io_utilization = (double)io_busy_time/curr_time;
    double avg_turnaround = (double)tot_turnaround_time/total_ios;
    double avg_waittime = (double)tot_wait_time/total_ios;
    printf("SUM: %d %d %.4lf %.2lf %.2lf %d\n",curr_time, total_movement, io_utilization, avg_turnaround, avg_waittime, maxwaittime);
    return 0;
}