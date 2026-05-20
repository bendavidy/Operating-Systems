
#include <stdlib.h>
// TODO: delete this
#include <stdio.h>
#include "os.h"

int has_valid_pte(uint64_t *node){
    for (int i = 0; i < 512; i++){
        if ((node[i] & 1) == 1){
            return 1;
        }
    }
    return 0;
}

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn){
    /*
    A function to create/destroy virtual memory mappings in a page table.
    This function takes the following arguments:
    (a) pt: The physical page number of the page table root (this is the physical page that the page table base register in the CPU state will point to).
    You can assume that pt has been previously returned by alloc page frame().
    (b) vpn: The virtual page number the caller wishes to map/unmap.
    (c) ppn: Can be one of two cases. If ppn is equal to a special NO_MAPPING value (defined in os.h), then vpn’s mapping (if it exists) should be destroyed.
    Otherwise, ppn specifies the physical page number that vpn should be mapped to
    */

    int isValid = 1;
    uint64_t *node, pte, index, new_page;
    node = phys_to_virt(pt);
    // save the parents pte in an array
    uint64_t** parent_nodes = malloc(5 * sizeof(uint64_t*));

    for (int i = 0; i < 5; i++) {
        index = (vpn >> (9 * (4 - i))) & 0x7f;
        pte = node[index];
        
        // invalid PTE
        if ((pte & 1) == 0) {
            isValid = 0;
        }
        
        if (isValid) {
            node = phys_to_virt(pte >> 12);
        }
        else if (ppn != NO_MAPPING) {
            // allocate a new page and point to the next level
            new_page = alloc_page_frame();
            node[index] = (new_page << 12) + 1;
            node = phys_to_virt(new_page);
        } else {
            // not valid and ppn=NO_MAPPING
            free(parent_nodes);
            return;
        }
        parent_nodes[i] = node;
    }

    if (ppn == NO_MAPPING) {
        // go over to invalidate parents if necessary
        for (int i = 4; i >= 0; i--){
            index = (vpn >> (9 * (4 - i))) & 0x7f;
            node = parent_nodes[i];
            node[index] = (node[index] >> 1) << 1;
            if (has_valid_pte(node)) {
                break;
            }
        }
    }
    free(parent_nodes);
}

uint64_t page_table_query(uint64_t pt, uint64_t vpn){
    /*
    A function to query the mapping of a virtual page number in a page table.
    This function returns the physical page number that vpn is mapped to, or NO_MAPPING if no mapping exists.
    The meaning of the pt argument is the same as with page table update().
    */

   // our pt has 5 levels
    uint64_t *node, pte;
    printf("here");
    node = phys_to_virt(pt);
    printf("here");
    for (int i = 0; i < 5; i++) {
        pte = node[(vpn >> (9 * (4 - i))) & 0x7f];

        // invalid PTE
        if ((pte & 1) == 0){
            return NO_MAPPING;
        }

        node = phys_to_virt(pte >> 12);
    }
    return pte >> 12;

    // 1. (vpn >> 36) & 0x7f
    // 2. (vpn >> 27) & 0x7f
    // 3. (vpn >> 18) & 0x7f
    // 4. (vpn >> 9) & 0x7f
    // 5. (vpn >> 0) & 0x7f
}



/*
Use the following function to allocate a physical page (also called page frame):
    uint64_t alloc_page_frame(void);
This function returns the physical page number of the allocated page. In this assignment, you do not need to free physical pages.
If alloc page frame() is unable to allocate a physical page, it will exit the program.
The content of the allocated page frame is all zeroes

Use the following function to obtain a pointer (i.e., kernel virtual address) to a physical address:
    void* phys_to_virt(uint64_t phys_addr);
The valid inputs to phys to virt() are addresses that reside in physical pages that were previously returned by alloc page frame().
If it is called with an invalid input, it returns NULL
*/