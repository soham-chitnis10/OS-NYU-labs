#include "include/process.h"
#include <cstring>

VMA::VMA(uint32_t s, uint32_t e, uint32_t w, uint32_t f) {
    start_vpage = s;
    end_vpage = e;
    file_mapped = f;
    write_protected = w;
}

Process::Process(uint64_t p_id) {
    proc_id = p_id;
    unmaps = maps = ins = outs = fins = fouts = zeros = segv = segprot = 0;
    memset(page_table, 0, sizeof(pte_t) * 64);
}
