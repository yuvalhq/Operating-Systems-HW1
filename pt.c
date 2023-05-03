#include "os.h"

#define OFFSET 12 // according to x86 architecture
#define PT_LEVELS 4 // according to x86 architecture 
#define LEVEL_SHIFT 9 // according to x86 architecture
#define KEEP_FIRST_9_BITS(x) (x & 0b111111111) // with "&" operator, this will only keep the first 9 bits a turn others to 0s
#define NOT_VALID(x) ((x & 0b000000001) == 0) // checking if bit 0 is not valid (is 0)

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn){
    int lvl;
    uint64_t *pt_lvl_pointer;
    uint64_t idx, pt_pn = pt << OFFSET; // pt_pn: page table's page number

    for (lvl = PT_LEVELS; lvl > 0; lvl--) {
        pt_lvl_pointer = phys_to_virt(pt_pn);
        idx = vpn >> (lvl * LEVEL_SHIFT);
        idx = KEEP_FIRST_9_BITS(idx);

        if (NOT_VALID(pt_lvl_pointer[idx])) {
            if (ppn == NO_MAPPING) {
                // at this point, there is no mapping and no need to map!
                return;
            }
            // at this point, there is no mapping and we need to allocate mem for mapping
            pt_lvl_pointer[idx] = (alloc_page_frame() << OFFSET) + 1;
        }
        // getting the info to update the next pt level
        pt_pn = pt_lvl_pointer[idx] - 1;
    }

    pt_lvl_pointer = phys_to_virt(pt_pn);
    idx = KEEP_FIRST_9_BITS(vpn);
    if (ppn == NO_MAPPING) {
        // at this point, there is a mapping and we need to remove it
        pt_lvl_pointer[idx] = 0;
    }
    else {
       // at this point, there is a mapping and add the ppn to trie struct
       pt_lvl_pointer[idx] = (ppn << OFFSET) + 1;
    }

    return;
}

uint64_t page_table_query(uint64_t pt, uint64_t vpn) {
    int lvl;
    uint64_t *pt_lvl_pointer;
    uint64_t idx, pt_pn = pt << OFFSET; // pt_pn: page table's page number
    
    for (lvl = PT_LEVELS; lvl > -1; lvl--) {
        pt_lvl_pointer = phys_to_virt(pt_pn);
        idx = vpn >> (lvl * LEVEL_SHIFT);
        idx = KEEP_FIRST_9_BITS(idx);

        if (NOT_VALID(pt_lvl_pointer[idx])) {
            // at this point, we found out there is no mapping!
            return NO_MAPPING;
        }
        // getting the info to search the next pt level
        pt_pn = pt_lvl_pointer[idx] - 1;
    }

    // return the leaf value from trie == the vpn adress + add back the offset (of 0s)
    return pt_lvl_pointer[idx] >> OFFSET;
}
