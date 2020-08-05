#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H


// Any changes to this file will be discarded for grading.
// Typically, the MMU is implemented in hardware, so there is a specific page table format that cannot be changed.
// When the user process wants to read or write to a page, the MMU will do the following:
// 1. If the page of interest does not have 'valid' bit set, the MMU will raise SIGUSR1 to your OS, and block until you raise SIGCONT to the MMU.
// 2. Otherwise, the MMU will set 'referenced' bit.  If the user process is writing to the page, the MMU will additionally set 'dirty' bit.
// 3. The MMU will use the 'frame_index' specified to read/write data to that frame in RAM, as directed by the user process.

#define FRAME_BITS GRADER_FRAME_BITS
#define PAGE_BITS GRADER_PAGE_BITS

typedef struct {
    unsigned frame_index : FRAME_BITS;
    unsigned valid : 1; // if the user program attempts to write to the page when valid==0, the requested_page field in the page_table will be set to the index of this page_table_entry, and the SIGUSR1 signal will be raised to your OS simulator.
    unsigned referenced : 1; // will be set by the MMU when the user program reads or writes to this page
    unsigned dirty : 1; // will be set by the MMU when the user program writes to this page
} page_table_entry;

typedef struct {
    page_table_entry entries[1<<PAGE_BITS];
} page_table;


#endif
