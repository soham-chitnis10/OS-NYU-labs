#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <vector>
#include <algorithm>
#include "ioreq.h"
#include "iosched.h"

bool v_flag = false;
bool f_flag = false;
bool q_flag = false;

int total_time = 0;
int total_movement = 0;
int curr_time = 1;
int curr_track = 0;
int total_ios = 0;
bool check_completed = true;

int main(int argc, char *argv[]) {
    char algo = 'N';
    int op;
    while ((op = getopt(argc, argv, "s:vqf")) != -1) {
        switch (op) {
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
    IOSchedBase *io_sched;
    switch (algo) {
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
    char *inputfile = argv[argc - 1];
    FILE *fp = fopen(inputfile, "r");
    char *str = NULL;
    size_t n = 0;
    while (getline(&str, &n, fp) != -1) {
        if (str[0] == '#') continue;
        int arr_time, track_num;
        sscanf(str, "%d %d", &arr_time, &track_num);
        IOReq *req = new IOReq(total_ios++, arr_time, track_num);
        IOs.push_back(req);
    }
    if (v_flag) printf("TRACE\n");
    while (true) {
        if (io_add_ptr < (int)IOs.size() && curr_time == IOs[io_add_ptr]->arr_time) {
            io_sched->add_req(IOs[io_add_ptr]);
            if (v_flag) printf("%d: %5d add %d\n", curr_time, IOs[io_add_ptr]->id, IOs[io_add_ptr]->track_num);
            io_add_ptr++;
        }
        while (check_completed) {
            if (active) {
                if (active->track_num == curr_track) {
                    active->end_time = curr_time;
                    if (v_flag) printf("%d: %5d finish %d\n", curr_time, active->id, active->end_time - active->arr_time);
                    io_sched->completed_req();
                    active = nullptr;
                } else {
                    check_completed = false;
                }
            }
            if (active == nullptr) {
                active = io_sched->strategy();
                if (active == nullptr) break;
                active->start_time = curr_time;
                if (q_flag) io_sched->print_queue();
                if (v_flag) printf("%d: %5d issue %d %d\n", curr_time, active->id, active->track_num, curr_track);
            }
        }
        if (active) {
            if (curr_track < active->track_num)
                curr_track++;
            else
                curr_track--;
            total_movement++;
        }
        if (io_sched->all_processed() && io_add_ptr == total_ios) break;
        curr_time++;
        check_completed = true;
    }
    int io_busy_time = 0;
    int tot_turnaround_time = 0;
    int tot_wait_time = 0;
    int maxwaittime = INT_MIN;
    for (auto io : IOs) {
        printf("%5d: %5d %5d %5d\n", io->id, io->arr_time, io->start_time, io->end_time);
        io_busy_time += (io->end_time - io->start_time);
        tot_turnaround_time += (io->end_time - io->arr_time);
        tot_wait_time += (io->start_time - io->arr_time);
        if ((io->start_time - io->arr_time) > maxwaittime)
            maxwaittime = (io->start_time - io->arr_time);
    }
    double io_utilization = (double)io_busy_time / curr_time;
    double avg_turnaround = (double)tot_turnaround_time / total_ios;
    double avg_waittime = (double)tot_wait_time / total_ios;
    printf("SUM: %d %d %.4lf %.2lf %.2lf %d\n", curr_time, total_movement, io_utilization, avg_turnaround, avg_waittime, maxwaittime);
    return 0;
}
