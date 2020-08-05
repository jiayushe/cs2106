/*************************************
 * Lab 4 Exercise 2
 * Name: 
 * Student No:
 * Lab Group:
 *************************************/

// You can modify anything in this file.
// You can also add any global or local variables you need (e.g. to implement your page replacement algorithm).

#include <signal.h>
#include <stdlib.h>
#include <assert.h>

#include "api.h"

typedef struct {
    unsigned mapped : 1;
    unsigned on_disk : 1;
} page_attr;

void os_run(int initial_num_pages, page_table *pg_table){
    // The main loop of your memory manager
    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGUSR1);
    sigaddset(&signals, SIGUSR2);
    
    int const total_frames = (1<<FRAME_BITS);
    int *page_in_frame = malloc(sizeof(int) * total_frames); // which page does the given frame contain (-1 if it does not exist)
    int *page_unmapped_buffer = malloc(sizeof(int) * (1<<PAGE_BITS)); // linked list of unmapped pages, terminated with -1
    int page_unmapped_list;
    page_attr *pg_attr = malloc(sizeof(page_attr) * (1<<PAGE_BITS)); // boolean array, 1 if page is mapped, 0 otherwise
    int next_victim_frame = 0;
    
    // create the pages in the disk first, because every page must be backed by the disk for this lab
    for (int i=0; i!=initial_num_pages; ++i) {
        pg_attr[i].mapped = 1;
        pg_attr[i].on_disk = 0;
    }
    
    for (int i=initial_num_pages; i!=(1<<PAGE_BITS); ++i){
        page_unmapped_buffer[i] = i+1;
        pg_attr[i].mapped = 0;
        pg_attr[i].on_disk = 0;
    }
    for (int i=0;i!=(1<<FRAME_BITS);++i){
        page_in_frame[i] = -1;
    }
    page_unmapped_buffer[(1<<PAGE_BITS)-1]=-1;
    page_unmapped_list = initial_num_pages;
    
    while (1) {
        siginfo_t info;
        sigwaitinfo(&signals, &info);
        
        if (info.si_signo == SIGUSR1) {
            // retrieve the index of the page that the user program wants, or -1 if the user program has terminated
            int const requested_page = info.si_value.sival_int;
            
            if (requested_page == -1) break;
            
            int segfault;
            
            if (!pg_attr[requested_page].mapped) {
                // page is not mapped
                segfault = 1; 
            }
            else {
                segfault = 0;
                
                int target_page;
                
                while (1) {
                    target_page = page_in_frame[next_victim_frame];
                    if (target_page == -1 || pg_table->entries[target_page].referenced == 0) {
                        // found the page to evict
                        break;
                    }
                    pg_table->entries[target_page].referenced = 0;
                    if (++next_victim_frame == total_frames) next_victim_frame = 0;
                }
                if(target_page != -1 && pg_attr[target_page].mapped) {
                    if (pg_table->entries[target_page].dirty) {
                        // if dirty, need to write to disk
                        disk_write(next_victim_frame, target_page);
                    }
                    // update page table
                    pg_table->entries[target_page].valid = 0;
                }
                
                // read new page
                if (!pg_attr[requested_page].on_disk) {
                    disk_create(requested_page);
                    pg_attr[requested_page].on_disk = 1;
                }
                disk_read(next_victim_frame, requested_page);
                page_in_frame[next_victim_frame] = requested_page;
                
                // update page table
                page_table_entry *entry = pg_table->entries + requested_page;
                entry->frame_index = next_victim_frame;
                entry->valid = 1;
                entry->referenced = 0;
                entry->dirty = 0;
                
                if (++next_victim_frame == total_frames) next_victim_frame = 0;
            }
            
            // tell the MMU that we are done updating the page table
            union sigval reply_value;
            reply_value.sival_int = segfault; // set to 0 if the page is successfully loaded, set to 1 if the page is not mapped to the user process (i.e. segfault)
            sigqueue(info.si_pid, SIGCONT, reply_value);
        }
        else if (info.si_signo == SIGUSR2) {
            // retrieve the index of the page that the user program wants, or -1 if the user program has terminated
            int const requested_page = info.si_value.sival_int;
            
            if (requested_page == -1) {
                // mmap
                assert(page_unmapped_list != -1); // assert that we have enough pages remaining
                int newpagenum = page_unmapped_list;
                page_unmapped_list = page_unmapped_buffer[page_unmapped_list];
                pg_attr[newpagenum].mapped = 1;
                pg_attr[newpagenum].on_disk = 0;
                
                // tell the MMU that we are done
                union sigval reply_value;
                reply_value.sival_int = newpagenum;
                sigqueue(info.si_pid, SIGCONT, reply_value);
            }
            else {
                // munmap
                if (pg_attr[requested_page].mapped) { // page is currently mapped
                    page_unmapped_buffer[requested_page] = page_unmapped_list;
                    page_unmapped_list = requested_page;
                    pg_attr[requested_page].mapped = 0;
                    page_table_entry *entry = pg_table->entries + requested_page;
                    if (pg_attr[requested_page].on_disk) {
                        disk_delete(requested_page);
                        pg_attr[requested_page].on_disk = 0;
                    }
                    if (entry->valid) {
                        // if the page is currently mapped to a frame, the page should be marked as invalid
                        page_in_frame[entry->frame_index] = -1;
                        entry->valid = 0;
                    }
                }
                
                // tell the MMU that we are done
                union sigval reply_value;
                reply_value.sival_int = 0; // dummy value, unused in runner
                sigqueue(info.si_pid, SIGCONT, reply_value);
            }
        }
    }
    
    free(pg_attr);
    free(page_unmapped_buffer);
    free(page_in_frame);
}
    