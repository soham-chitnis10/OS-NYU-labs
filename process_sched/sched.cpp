#include <stdio.h>
#include <iostream>
#include<bits/stdc++.h>
#define TRANS_TO_READY 0
#define TRANS_TO_RUN 1
#define TRANS_TO_PREEMPT 2
#define TRANS_TO_BLOCK 3
typedef enum{ STATE_CREATED, STATE_READY, STATE_RUNNING , STATE_BLOCKED, STATE_DONE} State;
using namespace std;


class Process
{
    public:
    State state;
    int state_ts;
    int static_prio;
    int dynamic_prio;
    int total_cpu_time;
    int at;
    int CB;
    int IO;
    int id;
    int ft;
    int it;
    int cw;
    int cpu_time;
    int current_cb;
    Process(State s,int st_ts, int sprio, int dprio, int tot_cpu, int cb, int io, int ID, int AT, int FT, int IT,int CW, int cpu_time_init, int ccb)
    {
        state = s;
        state_ts = st_ts;
        static_prio = sprio;
        dynamic_prio = dprio;
        total_cpu_time = tot_cpu;
        CB = cb;
        IO = io;
        id = ID;
        at = AT;
        ft = FT;
        it = IT;
        cw = CW;
        cpu_time = cpu_time_init;
        current_cb = ccb;
    }
};

int CURRENT_TIME = 0;
int IO_BUSY_TIME = 0;
bool CALL_SCHEDULER = false;
Process * CURRENT_RUNNING_PROCESS = nullptr;
long * randvals;
bool vflag = false;
bool tflag = false;
bool eflag = false;
bool pflag = false;
bool IO_USE = false;
int IO_started_at = -1;
int IO_lasts_till = -1;
int ofs=0;
class Event
{
    public:
    Process * evtProcess;
    int evtTimeStamp;
    int transition;
    Event(Process * proc,int time,int t)
    {
        evtProcess = proc;
        evtTimeStamp = time;
        transition = t;
    }
};

list<Event*> event_queue;
using eventqit = list<Event*>::iterator;
map<int,Process *> proc_info;
Event * get_event()
{
    if(event_queue.empty())
    {
        return nullptr;
    }
    Event * evt = event_queue.front();
    event_queue.pop_front();
    return evt;
}
int myrandom(int burst)
{ 
    return 1 + (randvals[ofs++] % burst); 
}
void display_eventQ()
{
    for(eventqit itr =  event_queue.begin();itr!=event_queue.end();itr++)
    {
        printf("%d:%d:%d ", (*itr)->evtTimeStamp,(*itr)->evtProcess->id,(*itr)->evtProcess->state);
    }
}
class Scheduler
{
    public:
    int quantum;
    virtual Process * get_next_process() = 0;
    virtual void add_process(Process *) = 0;
    virtual void display() = 0;
    virtual bool test_preempt(Process *) = 0;
};

class FCFS : public Scheduler
{
    private: deque<Process*> runqueue;
    public:
    Process * get_next_process()
    {
        if(runqueue.empty())
        {
            return nullptr;
        }
        Process * proc = runqueue.front();
        runqueue.pop_front();
        return proc;
    }
    void add_process(Process * proc)
    {
        runqueue.push_back(proc);
    }
    void display()
    {
        for(auto it: runqueue)
        {
            printf("%d:%d ",it->id,it->state);
        }
        cout<<"\n";
    }
    bool test_preempt(Process * proc)
    {
        return false;
    }
    FCFS()
    {
        quantum=10000;
    }
    FCFS(int q)
    {
        quantum = q;
    }

};

class LCFS : public Scheduler
{
    private: deque<Process*> runqueue;
    public:
    Process * get_next_process()
    {
        if(runqueue.empty())
        {
            return nullptr;
        }
        Process * proc = runqueue.back();
        runqueue.pop_back();
        return proc;
    }
    void add_process(Process * proc)
    {
        runqueue.push_back(proc);
    }
    void display()
    {
        for(auto it: runqueue)
        {
            printf("%d:%d ",it->id,it->state);
        }
        cout<<"\n";
    }
    bool test_preempt(Process * proc)
    {
        return false;
    }
    LCFS()
    {
        quantum=10000;
    }

};

class SRTF : public Scheduler
{
    private: list<Process*> runqueue;
    public:
    Process * get_next_process()
    {
        if(runqueue.empty())
        {
            return nullptr;
        }
        Process * proc = runqueue.front();
        runqueue.pop_front();
        return proc;
    }
    list<Process*>::iterator find_process_it(int rem)
    {
        auto itr = runqueue.begin();
        for(;itr!=runqueue.end();itr++)
        {
            int it_rem_time = (*itr)->total_cpu_time - (*itr)->cpu_time;
            if(it_rem_time > rem)
            {
                return itr;
            }
        }
        return runqueue.end();
    }
    void add_process(Process * proc)
    {
        int rem = proc->total_cpu_time - proc->cpu_time;
        auto it = find_process_it(rem);
        if(it!=runqueue.end())
        runqueue.insert(it,proc);
        else
        runqueue.push_back(proc);
    }
    void display()
    {
        for(auto it: runqueue)
        {
            printf("%d:%d ",it->id,it->state);
        }
        cout<<"\n";
    }
    bool test_preempt(Process * proc)
    {
        return false;
    }
    SRTF()
    {
        quantum=10000;
    }

};

class PRIO : public Scheduler
{
    protected:
    vector<deque<Process*>> * activequeue_ptr;
    vector<deque<Process*>> * expiredqueue_ptr;
    int max_prios;
    public:

    bool check_empty(vector<deque<Process*>> * ptr)
    {
        for(int i = max_prios - 1;i>=0;i--)
        {
            if(!ptr->at(i).empty())
            {
                return false;
            }
        }
        return true;
    }
    Process * get_next_process()
    {
        Process * proc;
        if(check_empty(activequeue_ptr) && check_empty(expiredqueue_ptr))
        return nullptr;
        if(!check_empty(activequeue_ptr))
        {
            for( int i = max_prios -1;i>=0;i--)
            {
                if(!activequeue_ptr->at(i).empty())
                {
                    proc = activequeue_ptr->at(i).front();
                    activequeue_ptr->at(i).pop_front();
                    break;
                }
            }
        }
        else
        {
            vector<deque<Process*>> * swap = activequeue_ptr;
            activequeue_ptr = expiredqueue_ptr;
            expiredqueue_ptr = swap;
            return get_next_process();
        }
        return proc;
    }
    void add_process(Process * proc)
    {
        if(proc->dynamic_prio < 0)
        {
            proc->dynamic_prio = proc->static_prio -1;
            expiredqueue_ptr->at(proc->dynamic_prio).push_back(proc);
        }
        else
        {
            activequeue_ptr->at(proc->dynamic_prio).push_back(proc);
        }
    }
    void display()
    {
        cout<<"ActiveQ:\n";
        for(int i = max_prios-1;i>=0;i--)
        {
            cout<<"Priority: "<<i<<endl;
            for(auto it: activequeue_ptr->at(i))
            {
                printf("%d:%d ",it->id,it->state);
            }
            cout<<"\n";
        }
        cout<<"ExpiredQ:\n";
        for(int i = max_prios-1;i>=0;i--)
        {
            cout<<"Priority: "<<i;
            for(auto it: expiredqueue_ptr->at(i))
            {
                printf("%d:%d ",it->id,it->state);
            }
            cout<<"\n";
        }
    }
    bool test_preempt(Process * proc)
    {
        return false;
    }
    PRIO()
    {}
    PRIO(int q, int mprios)
    {
        quantum = q;
        max_prios = mprios;
        activequeue_ptr = new vector<deque<Process*>>(max_prios);
        expiredqueue_ptr = new vector<deque<Process*>>(max_prios);
    }

};

class EPRIO: public PRIO
{
    public:
    bool test_preempt(Process * proc)
    {
        if(CURRENT_RUNNING_PROCESS == nullptr)
        {
            return false;
        }
        int next_event_time_current=-1;
        Process * p;
        for(eventqit itr = event_queue.begin();itr!=event_queue.end();itr++)
        {
            
            if((*itr)->evtProcess->id == CURRENT_RUNNING_PROCESS->id)
            {
                next_event_time_current = (*itr)->evtTimeStamp;
                break;
            }
        }
        return (proc->dynamic_prio > CURRENT_RUNNING_PROCESS->dynamic_prio) && (next_event_time_current > CURRENT_TIME);
    }
    EPRIO(int q, int mprios)
    {
        quantum = q;
        max_prios = mprios;
        activequeue_ptr = new vector<deque<Process*>>(max_prios);
        expiredqueue_ptr = new vector<deque<Process*>>(max_prios);
    }
};
int get_next_event_time()
{
    if(event_queue.empty())
    {
        return -1;
    }
    return event_queue.front()->evtTimeStamp;
}

eventqit find_it(int time)
{
    eventqit itr = event_queue.begin();
    for(;itr!=event_queue.end();itr++)
    {
        if((*itr)->evtTimeStamp > time)
        return itr;
    }
    return event_queue.end();
}


void add_event(Process * proc, int time, int transition)
{
    eventqit itr = find_it(time);
    if(itr!= event_queue.end())
    event_queue.insert(itr,new Event(proc,time, transition));
    else
    event_queue.push_back(new Event(proc,time, transition));
}

void display_summary()
{
    int cpu_busy_time = 0;
    int num_processes = proc_info.size();
    int finish_time = CURRENT_TIME;
    int total_tt = 0;
    int total_cw = 0;
    for(auto it: proc_info)
    {
        printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n", it.first, it.second->at, it.second->total_cpu_time, it.second->CB, it.second->IO, it.second->static_prio,it.second->ft, (it.second->ft - it.second->at), it.second->it, it.second->cw);
        cpu_busy_time += it.second->total_cpu_time;
        total_tt += (it.second->ft - it.second->at);
        total_cw += it.second->cw;
    }
    double cpu_util = 100.0 * (cpu_busy_time / (double) finish_time);
    double io_util = 100.0 * (IO_BUSY_TIME / (double) finish_time);
    double avg_tt = (total_tt / (double) num_processes);
    double avg_cw = (total_cw / (double) num_processes);
    double throughput = 100.0 * (num_processes / (double) finish_time);
    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",finish_time,cpu_util,io_util,avg_tt,avg_cw, throughput);
}

void Simulation(Scheduler * sched) 
{
    Event* evt;
    while( (evt = get_event()) ) 
    {
        Process *proc = evt->evtProcess;
        // this is the process the event works on
        CURRENT_TIME = evt->evtTimeStamp;
        int transition = evt->transition;
        // for accouting
        int timeInPrevState = CURRENT_TIME - proc->state_ts;
        delete evt; evt = nullptr; // remove cur event obj and don’t touch anymore
        switch (transition) 
        { // encodes where we come from and where we go
            case TRANS_TO_READY:
            // must come from BLOCKED or CREATED
            // must add to run queue, no event created
            if(vflag)
            {
                printf("%d %d %2d: %d -> %d\n",CURRENT_TIME, proc->id,timeInPrevState, proc->state, STATE_READY);
            }
            if(proc->state == STATE_BLOCKED)
            {
                proc->it += timeInPrevState;
                proc->dynamic_prio = proc->static_prio -1;

                if(IO_lasts_till==CURRENT_TIME && IO_USE)
                {
                    IO_USE=false;
                    IO_BUSY_TIME += (IO_lasts_till - IO_started_at);
                }
            }
            proc->state = STATE_READY;
            proc->state_ts = CURRENT_TIME;
            sched->add_process(proc);
            if(sched->test_preempt(proc))
            {
                if(eflag)
                {
                    printf("RemoveEvent(%d): ",CURRENT_RUNNING_PROCESS->id);
                    display_eventQ();
                }
                eventqit i = event_queue.begin();
                for(;i!=event_queue.end();i++)
                {
                    if((*i)->evtProcess->id == CURRENT_RUNNING_PROCESS->id)
                    {
                        break;
                    }
                }
                event_queue.erase(i);
                if(eflag)
                {
                    printf(" ==> ");
                    display_eventQ();
                }
                if(eflag)
                {
                    printf("AddEvent(%d:%d:%d): ", CURRENT_TIME,CURRENT_RUNNING_PROCESS->id,TRANS_TO_PREEMPT);
                    display_eventQ();
                    printf("==> ");

                }
                add_event(CURRENT_RUNNING_PROCESS,CURRENT_TIME,TRANS_TO_PREEMPT);
                if(eflag)
                {
                    display_eventQ();
                    printf("\n");
                }
            }
            CALL_SCHEDULER = true;
            // event_queue.pop_front();
            break;
            case TRANS_TO_PREEMPT: // similar to TRANS_TO_READY
            // must come from RUNNING (preemption)
            // must add to run queue, no event created
            proc->cpu_time += timeInPrevState;
            proc->current_cb -= timeInPrevState;
            if(vflag)
            {
                printf("%d %d %2d: %d -> %d cb=%d rem=%d prio=%d\n",CURRENT_TIME, proc->id,timeInPrevState, proc->state, STATE_READY, proc->current_cb,proc->total_cpu_time - proc->cpu_time,proc->dynamic_prio);
            }
            proc->state_ts = CURRENT_TIME;
            proc->state = STATE_READY;
            proc->dynamic_prio -=1;
            CURRENT_RUNNING_PROCESS = nullptr;
            sched->add_process(proc);
            CALL_SCHEDULER = true;
            break;
            case TRANS_TO_RUN:
            // create event for either preemption or blocking
            
            if(proc->current_cb == 0)
            {
                proc->current_cb= min(myrandom(proc->CB),proc->total_cpu_time - proc->cpu_time);
            }    
            
            if(vflag)
            {
                printf("%d %d %2d: %d -> %d cb=%d rem=%d prio=%d\n",CURRENT_TIME, proc->id,timeInPrevState, proc->state, STATE_RUNNING,proc->current_cb,proc->total_cpu_time - proc->cpu_time, proc->dynamic_prio);
            }
            proc->cw += timeInPrevState;
            proc->state = STATE_RUNNING;
            proc->state_ts = CURRENT_TIME;
            if(proc->current_cb > sched->quantum)
            {
                if(eflag)
                {
                    printf("AddEvent(%d:%d:%d): ", CURRENT_TIME+sched->quantum,proc->id,TRANS_TO_PREEMPT);
                    display_eventQ();
                    printf("==> ");

                }
                add_event(proc,CURRENT_TIME+sched->quantum,TRANS_TO_PREEMPT);
                if(eflag)
                {
                    display_eventQ();
                    printf("\n");
                } 
            }
            else
            {
                int new_event_time = (CURRENT_TIME+proc->current_cb);
                if(eflag)
                {
                    printf("AddEvent(%d:%d:%d): ",CURRENT_TIME,(CURRENT_TIME+proc->current_cb),TRANS_TO_BLOCK );
                    display_eventQ();
                    printf("==> ");

                }
                add_event(proc,new_event_time,TRANS_TO_BLOCK);
                if(eflag)
                {
                    display_eventQ();
                    printf("\n");
                } 
            }
        
            break;
            case TRANS_TO_BLOCK:
            //create an event for when process becomes READY again
            proc->current_cb = 0;
            proc->cpu_time += timeInPrevState;
            CURRENT_RUNNING_PROCESS = nullptr;
            if((proc->total_cpu_time - proc->cpu_time) > 0)
            {
                int io_burst = myrandom(proc->IO);
                if(vflag)
                {
                    printf("%d %d %2d: %d -> %d ib=%d rem=%d\n",CURRENT_TIME, proc->id,timeInPrevState, proc->state, STATE_BLOCKED,io_burst, proc->total_cpu_time - proc->cpu_time);
                }
                if(eflag)
                {
                    printf("AddEvent(%d:%d:%d): ", CURRENT_TIME+io_burst,proc->id,TRANS_TO_READY);
                    display_eventQ();
                    printf("==> ");
                }
                proc->state_ts = CURRENT_TIME;
                proc->state = STATE_BLOCKED;
                add_event(proc,CURRENT_TIME+io_burst,TRANS_TO_READY);
                if(eflag)
                {
                    display_eventQ();
                    printf("\n");
                }
                if(IO_USE)
                {
                    IO_lasts_till = max(IO_lasts_till,CURRENT_TIME+io_burst);
                }
                else
                {
                    IO_USE = true;
                    IO_started_at = CURRENT_TIME;
                    IO_lasts_till = CURRENT_TIME + io_burst;
                } 
            }
            else
            {
                proc->ft = CURRENT_TIME;
                proc->state = STATE_DONE;
                proc_info.insert(make_pair(proc->id,proc));
                if(vflag)
                {
                    printf("%d %d %2d: DONE\n",CURRENT_TIME, proc->id,timeInPrevState);
                }
            }
            CALL_SCHEDULER = true;
            break;
        }
        if(CALL_SCHEDULER) 
        {
            int next_tim = get_next_event_time();
            if ( next_tim== CURRENT_TIME)
                continue; //process next event from Event queue
            CALL_SCHEDULER = false; // reset global flag
            if (CURRENT_RUNNING_PROCESS == nullptr) 
            {
                CURRENT_RUNNING_PROCESS = sched->get_next_process();
                if (CURRENT_RUNNING_PROCESS == nullptr)
                continue;
                // create event to make this process runnable for same time.
                if(eflag)
                {
                    printf("AddEvent(%d:%d:%d): ", CURRENT_TIME,CURRENT_RUNNING_PROCESS->id, TRANS_TO_RUN);
                    display_eventQ();
                    printf("==> ");

                }
                add_event(CURRENT_RUNNING_PROCESS,CURRENT_TIME,TRANS_TO_RUN);
                if(eflag)
                {
                    display_eventQ();
                    printf("\n");
                }
            }
        }
    }
}


int main(int argc, char * argv[])
{
    int index;
    int op;
    char s;
    int quantum=10000;
    int maxprios =4;
    while( (op = getopt(argc,argv, "vteps:"))!= -1)
    {
        switch(op)
        {
            case 'v':
            vflag=true;
            break;
            case 't':
            tflag = true;
            break;
            case 'e':
            eflag = true;
            break;
            case 'p':
            pflag = true;
            break;
            case 's':
            sscanf(optarg,"%c%d:%d", &s, &quantum, &maxprios);
            break;
            case '?':
            printf("Unknown option `-%c'.\n", optopt); return 1;
            default: abort();
        }

    }
    char * inputfile =argv[argc-2];
    char * rfile =argv[argc-1];
    FILE* fp = fopen(rfile,"r");
    long tot_ran_num;
    char *str;
    size_t n;
    getline(&str,&n,fp);
    tot_ran_num = strtol(str, NULL,10);
    randvals = (long *)calloc(tot_ran_num,sizeof(long));
    for(long i=0;i<tot_ran_num;i++)
    {
        getline(&str,&n,fp);
        randvals[i] = strtol(str, NULL,10);
    }
    fclose(fp);
    fp = fopen(inputfile,"r");
    int procID = 0;
    while(getline(&str,&n,fp)!=-1)
    {
        int at, tcpu, cb, io;
        sscanf(str,"%d %d %d %d\n",&at,&tcpu,&cb, &io);
        int sprio = myrandom(maxprios);
        Process * proc =  new Process(STATE_CREATED,at,sprio,sprio-1,tcpu,cb,io,procID,at,-1,0,0,0,0);
        Event * evt = new Event(proc,at,TRANS_TO_READY);
        event_queue.push_back(evt);
        procID++;
    }
    if(eflag)
    {
        printf("ShowEventQ: ");
        for(auto it: event_queue)
        {
            printf(" %d:%d",it->evtTimeStamp,it->evtProcess->id);
        }
        printf("\n");
    }
    Scheduler * sched;
    switch(s)
    {
        case 'F':
        sched = new FCFS();
        Simulation(sched);
        cout<<"FCFS\n";
        display_summary();
        break;
        case 'L':
        sched = new LCFS();
        Simulation(sched);
        cout<<"LCFS\n";
        display_summary();
        break;
        case 'S':
        sched = new SRTF();
        Simulation(sched);
        cout<<"SRTF\n";
        display_summary();
        break;
        case 'R':
        sched = new FCFS(quantum);
        Simulation(sched);
        cout<<"RR "<<quantum<<"\n";
        display_summary();
        break;
        case 'P':
        sched = new PRIO(quantum,maxprios);
        Simulation(sched);
        cout<<"PRIO "<<quantum<<"\n";
        display_summary();
        break;
        case 'E':
        sched = new EPRIO(quantum,maxprios);
        Simulation(sched);
        cout<<"PREPRIO "<<quantum<<"\n";
        display_summary();
        break;
        default:
        perror("Not Implemented");
    }
    
    return 0;
}