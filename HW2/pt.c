
#include "os.h"
#include <stdio.h>
#include <stdlib.h>

uint64_t* check_ptr(void* a) {
    if (a == NULL) {
        printf("phys_to_virt returned a NULL pointer\n");
        exit(1);
    }
    return a;
}

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn) {
    int isValid = 1;
    uint64_t *node, pte, index, new_page;
    node = check_ptr(phys_to_virt(pt << 12));

    for (int i = 0; i < 4; i++) {
        index = (vpn >> (9 * (4 - i))) & 0x1ff; // isolating the 9 wanted bits
        pte = node[index];

        // invalid PTE
        if ((pte & 1) == 0) {
            isValid = 0;
        }
        
        if (isValid) {
            node = check_ptr(phys_to_virt(pte - 1)); // minus the valid bit
        }
        else if (ppn != NO_MAPPING) {
            // allocate a new page and point to the next level
            new_page = alloc_page_frame();
            node[index] = (new_page << 12) + 1;
            node = check_ptr(phys_to_virt(new_page << 12));
        } else {
            // not valid and ppn=NO_MAPPING
            return;
        }
    }

    // last level read
    index = vpn & 0x1ff; // isolating the first 9 bits

    if (ppn == NO_MAPPING) {
        // invalidate last pte
        node[index] = (node[index] >> 1) << 1;
    } else {
        // map ppn with an active bit
        node[index] = (ppn << 12) + 1;
    }
}

uint64_t page_table_query(uint64_t pt, uint64_t vpn) {
    uint64_t *node, pte;
    node = check_ptr(phys_to_virt(pt << 12));
    for (int i = 0; i < 4; i++) {
        pte = node[(vpn >> (9 * (4 - i))) & 0x1ff]; // isolating the 9 wanted bits
        // invalid PTE
        if ((pte & 1) == 0){
            return NO_MAPPING;
        }

        node = check_ptr(phys_to_virt(pte - 1)); // minus the valid bit
    }

    // last iteration
    pte = node[vpn & 0x1ff]; // isolating the 9 wanted bits
    if ((pte & 1) == 0) {
        return NO_MAPPING;
    }
    return pte >> 12;
}