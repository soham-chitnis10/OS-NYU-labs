#pragma once
#include <vector>
#include <cstdint>

struct pte_t {
    uint32_t valid : 1;
    uint32_t referenced : 1;
    uint32_t modified : 1;
    uint32_t write_protected : 1;
    uint32_t pagedout: 1;
    uint32_t frame_n :7;
    uint32_t pte : 20;
};

class VMA {
public:
    uint32_t start_vpage, end_vpage, write_protected, file_mapped;
    VMA(uint32_t s, uint32_t e, uint32_t w, uint32_t f);
};

class Process {
public:
    uint64_t proc_id;
    pte_t page_table[64];
    std::vector<VMA*> vmas;
    uint64_t unmaps, maps, ins, outs, fins, fouts, zeros, segv, segprot;
    Process(uint64_t p_id);
};
