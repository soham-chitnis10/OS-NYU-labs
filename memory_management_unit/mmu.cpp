bool O_flag = false;
bool F_flag = false;
bool P_flag = false;
bool S_flag = false;
bool x_flag = false;
bool y_flag = false;
bool f_flag = false;
bool a_flag = false;
typedef struct {
typedef struct {
// Modularized MMU implementation
#include "mmu.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <getopt.h>
using namespace std;

#define MAX_FRAMES 128
#define MAX_VPAGES 64
#define CTX_SWITCH_COST 130
#define EXIT_PROCESS_COST 1230
#define MAPS_COST 350
#define UNMAPS_COST 410
#define INS_COST 3200
#define OUTS_COST 2750
#define FINS_COST 2350
#define FOUTS_COST 2800
#define ZEROS_COST 150
#define SEGV_COST 440
#define SEGPROT_COST 410

int num_frames = 4;
bool O_flag = false, F_flag = false, P_flag = false, S_flag = false;
bool x_flag = false, y_flag = false, f_flag = false, a_flag = false;
long* randvals;
long ofs = 0;
long tot_ran_num;
uint64_t total_inst = 0;
unsigned long long total_cost = 0;
uint64_t ctx_switches = 0;
uint64_t process_exits = 0;

frame_t frame_table[MAX_FRAMES];
deque<frame_t*> free_frames;
vector<Process*> processes;
Process* current_process = nullptr;
Pager* pager;

int myrandom() {
    return (randvals[(ofs++) % tot_ran_num]);
}

// ...existing code...
    public:
    uint32_t start_vpage, end_vpage, write_protected, file_mapped;
    VMA(uint32_t s, uint32_t e, uint32_t w, uint32_t f)
    {
        start_vpage = s;
        end_vpage = e;
        file_mapped = f;
        write_protected = w;
    }
};

frame_t frame_table[MAX_FRAMES];
class Process{
    public:
    uint64_t proc_id;
    pte_t page_table[MAX_VPAGES];
    vector<VMA*> vmas;
    uint64_t unmaps, maps, ins, outs, fins, fouts, zeros, segv, segprot;

    Process(uint64_t p_id)
    {
        proc_id = p_id;
        unmaps = maps = ins = outs =  fins = fouts = zeros = segv = segprot = 0;
        memset(page_table,0,sizeof(pte_t)*MAX_VPAGES);
    }
};
vector<Process*> processes;
Process * current_process = nullptr;

class Pager{
    public:
    int hand;
    virtual frame_t* select_victim_frame() = 0;
};

class FIFO : public Pager{
    public:
    frame_t* select_victim_frame()
    {
        frame_t * frame = &(frame_table[(hand++)%num_frames]);
        return frame;
    }
    FIFO()
    {
        hand=0;
    }
};

class Random : public Pager{
    public:
    frame_t* select_victim_frame()
    {
        hand = myrandom()%num_frames;
        frame_t * frame = &(frame_table[hand]);
        return frame;
    }
    Random()
    {
        hand=0;
    }
};

class Clock : public Pager{
    public:
    frame_t* select_victim_frame()
    {
        while(true)
        {
            frame_t * frame = &(frame_table[(hand++)%num_frames]);
            if(!processes[frame->pid]->page_table[frame->vpage].referenced)
            {
                return frame;
            }
            else
            {
                processes[frame->pid]->page_table[frame->vpage].referenced=0;
            }
        }
    }
    Clock()
    {
        hand = 0;
    }
};

class NRU : public Pager{
    int last_inst_reset;
    int class_frames[4];
    public:
    frame_t* select_victim_frame()
    {
        frame_t * frame = nullptr;
        memset(class_frames,-1,sizeof(int)*4);
        bool reset = ((total_inst-last_inst_reset+1) >= 48);
        for(int i = 0;i<num_frames;i++)
        {
            frame_t * curr_frame = &(frame_table[(hand+i)%num_frames]);
            pte_t *pte = &(processes[curr_frame->pid]->page_table[curr_frame->vpage]);
            int class_id = pte->referenced*2+pte->modified;
            if(class_id==0 && class_frames[0]==-1)
            {
                frame = curr_frame;
                hand = (hand+i+1);
                break;
            }
            else if(class_frames[class_id]==-1)
            {
                class_frames[class_id] = curr_frame-frame_table;
            }
        }
        if(frame == nullptr)
        {
            for(int i =0;i<4;i++)
            {
                if(class_frames[i]!=-1)
                {
                    frame = &(frame_table[class_frames[i]]);
                    hand = class_frames[i]+1;
                    break;
                }
            }
        }
        if(reset)
        {
            for(int i =0;i<num_frames;i++)
            {
                pte_t * pte = &(processes[frame_table[i].pid]->page_table[frame_table[i].vpage]);
                pte->referenced=0;
            }
            last_inst_reset = total_inst+1;
        }
        return frame;
    }
    NRU()
    {
        last_inst_reset=0;
        hand=0;
    }
};

class Aging : public Pager{
    public:
    frame_t* select_victim_frame()
    {
        frame_t * frame  = &(frame_table[(hand)%num_frames]);
        for(int i = 0;i<num_frames;i++)
        {
            frame_t * curr_frame = &(frame_table[(hand+i)%num_frames]);
            curr_frame->age = curr_frame->age>>1;
            pte_t* pte = &(processes[curr_frame->pid]->page_table[curr_frame->vpage]);
            if(pte->referenced)
            {
                curr_frame->age = curr_frame->age | 0x80000000;
            }
            pte->referenced=0;
            if(curr_frame->age < frame->age)
            {
                frame = curr_frame;
            }
        }
        hand = frame-frame_table+1;
        return frame;
    }
    Aging()
    {
        hand=0;
    }
};

class WorkingSet : public Pager{
    public:
    int tau;
    frame_t* select_victim_frame()
    {
        uint32_t smallest_time = 0;
        frame_t* oldest_frame = nullptr;
        int start = hand;
        while(true)
        {
            frame_t * frame = &(frame_table[(hand++)%num_frames]);
            pte_t* pte = &(processes[frame->pid]->page_table[frame->vpage]);
            int age = (total_inst - frame->last_time_used);
            if(start%num_frames == frame-frame_table && (hand-1) > start)
            {
                if(oldest_frame == nullptr)
                    oldest_frame = frame;
                hand = oldest_frame-frame_table+1;
                return oldest_frame;
            }
            
            if(pte->referenced)
            {
                pte->referenced=0;
                frame->last_time_used = total_inst;
            }
            else if( age > tau)
            {
                return frame;
            }
            else if(age > smallest_time)
            {
                smallest_time=age;
                oldest_frame = frame;
            }
        }
    }
    WorkingSet()
    {
        tau = 49;
    }
};
Pager * pager;
deque<frame_t*> free_frames;
frame_t * allocate_frame_from_free_list()
{
    if(free_frames.empty())
    {
        return NULL;
    }
    frame_t * frame = free_frames.front();
    free_frames.pop_front();
    return frame;
}

frame_t *get_frame()
{
    frame_t *frame = allocate_frame_from_free_list();
    if(frame == NULL)
    {
        frame = pager->select_victim_frame();
    }
    return frame;
}
VMA * get_vma(vector<VMA*> vmas, uint32_t vpage)
{
    for(auto vma : vmas)
    {
        if(vpage >=vma->start_vpage && vpage <= vma->end_vpage)
        {
            return vma;
        }
    }
    return nullptr;
}
void print_frame_table()
{
    cout<<"FT:";
    for(int i = 0;i<num_frames;i++)
    {
        if(frame_table[i].pid!=-1)
        {
            printf(" %d:%d",frame_table[i].pid,frame_table[i].vpage);
        }
        else
        {
            printf(" *");
        }
    }
    cout<<"\n";
}
void print_page_table(int pid)
{
    printf("PT[%d]:", pid);
    for(int j =0;j<MAX_VPAGES;j++)
    {
        if(!processes[pid]->page_table[j].valid)
        {
            if(processes[pid]->page_table[j].pagedout)
            {
                printf(" #");
            }
            else
            {
                printf(" *");
            }
            continue;
        }
        printf(" %d:%c%c%c", j, (processes[pid]->page_table[j].referenced ? 'R': '-'), (processes[pid]->page_table[j].modified ? 'M' : '-'), (processes[pid]->page_table[j].pagedout ? 'S' : '-'));
    }
    cout<<"\n";
}
void print_all_page_tables(int num_process)
{
    for(int i = 0;i<num_process;i++)
    {
        print_page_table(i);
    }
}
void frame_init(frame_t * frame)
{
    frame->pid = -1;
    frame->vpage = -1;
    frame->age = 0;
    frame->last_time_used = 0;
}
int main(int argc, char* argv[]) {
    char algo = 'a';
    int op;
    while ((op = getopt(argc, argv, "f:a:o:")) != -1) {
        switch (op) {
            case 'f': sscanf(optarg, "%d", &num_frames); break;
            case 'a': sscanf(optarg, "%c", &algo); break;
            case 'o': {
                string s1 = optarg;
                for (auto ch : s1) {
                    switch (ch) {
                        case 'O': O_flag = true; break;
                        case 'F': F_flag = true; break;
                        case 'P': P_flag = true; break;
                        case 'S': S_flag = true; break;
                        case 'f': f_flag = true; break;
                        case 'x': x_flag = true; break;
                        case 'y': y_flag = true; break;
                        case 'a': a_flag = true; break;
                        default: break;
                    }
                }
                break;
            }
        }
    }
    char* inputfile = argv[argc - 2];
    char* rfile = argv[argc - 1];
    FILE* fp = fopen(rfile, "r");
    char* str = NULL;
    size_t n = 0;
    getline(&str, &n, fp);
    tot_ran_num = strtol(str, NULL, 10);
    randvals = (long*)calloc(tot_ran_num, sizeof(long));
    for (long i = 0; i < tot_ran_num; i++) {
        getline(&str, &n, fp);
        randvals[i] = strtol(str, NULL, 10);
    }
    fclose(fp);
    fp = fopen(inputfile, "r");
    uint64_t num_process;
    while (getline(&str, &n, fp) != -1) {
        if (str[0] != '#') break;
    }
    sscanf(str, "%ld", &num_process);
    for (uint64_t i = 0; i < num_process; i++) {
        while (getline(&str, &n, fp) != -1) {
            if (str[0] != '#') break;
        }
        uint64_t num_vmas;
        sscanf(str, "%ld", &num_vmas);
        Process* proc = new Process(i);
        for (uint64_t j = 0; j < num_vmas; j++) {
            while (getline(&str, &n, fp) != -1) {
                if (str[0] != '#') break;
            }
            uint32_t start_vpage, end_vpage, write_protected, file_mapped;
            sscanf(str, "%d %d %d %d", &start_vpage, &end_vpage, &write_protected, &file_mapped);
            VMA* vma = new VMA(start_vpage, end_vpage, write_protected, file_mapped);
            proc->vmas.push_back(vma);
        }
        processes.push_back(proc);
    }
    switch (algo) {
        case 'f': pager = new FIFO(); break;
        case 'r': pager = new Random(); break;
        case 'c': pager = new Clock(); break;
        case 'e': pager = new NRU(); break;
        case 'a': pager = new Aging(); break;
        case 'w': pager = new WorkingSet(); break;
        default: abort();
    }
    for (int i = 0; i < num_frames; i++) {
        frame_init(&frame_table[i]);
        free_frames.push_back(&frame_table[i]);
    }
    while (getline(&str, &n, fp) != -1) {
        if (str[0] == '#') continue;
        char inst; int arg;
        sscanf(str, "%c %d\n", &inst, &arg);
        if (O_flag) printf("%lu: ==> %c %d\n", total_inst, inst, arg);
        if (inst == 'e') {
            if (O_flag) printf("EXIT current process %d\n", arg);
            total_cost += EXIT_PROCESS_COST;
            Process* exited_proc = processes[arg];
            for (int i = 0; i < MAX_VPAGES; i++) {
                pte_t* pte = &(exited_proc->page_table[i]);
                if (pte->valid) {
                    frame_t* frame = &(frame_table[pte->frame_n]);
                    total_cost += UNMAPS_COST;
                    exited_proc->unmaps++;
                    if (O_flag) printf(" UNMAP %d:%d\n", arg, i);
                    VMA* v = get_vma(exited_proc->vmas, i);
                    if (pte->modified && v->file_mapped) {
                        exited_proc->fouts++;
                        if (O_flag) cout << " FOUT\n";
                        total_cost += FOUTS_COST;
                    }
                    frame_init(frame);
                    free_frames.push_back(frame);
                }
                memset((pte_t*)pte, 0, sizeof(pte_t));
            }
            process_exits++;
            total_inst++;
            continue;
        }
        if (inst == 'c') {
            current_process = processes[arg];
            ctx_switches++;
            total_cost += CTX_SWITCH_COST;
            total_inst++;
            continue;
        }
        pte_t* pte = &(current_process->page_table[arg]);
        VMA* v = nullptr;
        if (!pte->valid) {
            v = get_vma(current_process->vmas, arg);
            if (v == nullptr) {
                if (O_flag) cout << " SEGV\n";
                current_process->segv++;
                total_cost++;
                total_cost += SEGV_COST;
                total_inst++;
                continue;
            }
            pte->write_protected = v->write_protected;
            frame_t* newframe = get_frame();
            if (newframe->pid != -1 && newframe->vpage != -1) {
                uint32_t prev_vpage = newframe->vpage;
                uint32_t prev_proc = newframe->pid;
                pte_t* prev_proc_pte = &(processes[prev_proc]->page_table[prev_vpage]);
                prev_proc_pte->valid = 0;
                processes[prev_proc]->unmaps++;
                if (O_flag) printf(" UNMAP %d:%d\n", prev_proc, prev_vpage);
                total_cost += UNMAPS_COST;
                if (prev_proc_pte->modified) {
                    VMA* prev_vma = get_vma(processes[prev_proc]->vmas, prev_vpage);
                    prev_proc_pte->modified = 0;
                    if (prev_vma->file_mapped) {
                        if (O_flag) cout << " FOUT\n";
                        processes[prev_proc]->fouts++;
                        total_cost += FOUTS_COST;
                    } else {
                        if (O_flag) cout << " OUT\n";
                        processes[prev_proc]->outs++;
                        prev_proc_pte->pagedout = 1;
                        total_cost += OUTS_COST;
                    }
                }
            }
            if (v->file_mapped) {
                if (O_flag) cout << " FIN\n";
                current_process->fins++;
                total_cost += FINS_COST;
            } else if (pte->pagedout) {
                if (O_flag) {
                    cout << " IN\n";
                    current_process->ins++;
                }
                total_cost += INS_COST;
            } else {
                if (O_flag) cout << " ZERO\n";
                current_process->zeros++;
                total_cost += ZEROS_COST;
            }
            newframe->pid = current_process->proc_id;
            newframe->vpage = arg;
            newframe->age = 0;
            newframe->last_time_used = total_inst;
            pte->frame_n = newframe - frame_table;
            pte->valid = 1;
            current_process->maps++;
            if (O_flag) printf(" MAP %ld\n", newframe - frame_table);
            total_cost += MAPS_COST;
        }
        if (inst == 'r' || inst == 'w') {
            pte->referenced = 1;
            total_cost++;
            total_inst++;
        }
        if (inst == 'w') {
            if (pte->write_protected) {
                if (O_flag) cout << " SEGPROT\n";
                current_process->segprot++;
                total_cost += SEGPROT_COST;
            } else {
                pte->modified = 1;
            }
        }
        if (x_flag) print_page_table(current_process->proc_id);
        if (y_flag) print_all_page_tables(num_process);
        if (f_flag) print_frame_table();
    }
    if (P_flag) print_all_page_tables(num_process);
    if (F_flag) print_frame_table();
    if (S_flag) {
        for (auto proc : processes) {
            printf("PROC[%lu]: U=%lu M=%lu I=%lu O=%lu FI=%lu FO=%lu Z=%lu SV=%lu SP=%lu\n",
                proc->proc_id, proc->unmaps, proc->maps, proc->ins, proc->outs,
                proc->fins, proc->fouts, proc->zeros, proc->segv, proc->segprot);
        }
        printf("TOTALCOST %lu %lu %lu %llu %lu\n",
            total_inst, ctx_switches, process_exits, total_cost, sizeof(pte_t));
    }
    return 0;
}