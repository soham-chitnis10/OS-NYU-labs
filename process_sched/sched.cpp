#include <cstdio>
#include <iostream>
#include <getopt.h>
#include "include/process.h"
#include "include/event.h"
#include "include/scheduler.h"
#include "include/utils.h"
#define TRANS_TO_READY 0
#define TRANS_TO_RUN 1
#define TRANS_TO_PREEMPT 2
#define TRANS_TO_BLOCK 3
using namespace std;

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