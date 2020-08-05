/*************************************
 * Lab 4 Exercise 3
 * Name: She Jiayu
 * Student No: A0188314B
 * Lab Group: 9
 *************************************/

// You can modify anything in this file.
// Unless otherwise stated, a line of code being present in this template
//  does not imply that it is correct/necessary!
// You can also add any global or local variables you need (e.g. to implement your page replacement algorithm).

#include <signal.h>
#include <stdlib.h>

#include "api.h"

void os_run(int initial_num_pages, page_table *pg_table) {
    // The main loop of your memory manager
    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGUSR1);
    sigaddset(&signals, SIGUSR2);

    int i;
    // initialize pages
    int page_count = 1<<PAGE_BITS;
    int page_mapped[page_count];
    int page_avail[page_count];
    int cur_avail = initial_num_pages;
    for (i = 0; i != initial_num_pages; i++) {
        // create page in disk
        disk_create(i);
        page_mapped[i] = 1;
        page_avail[i] = -1;
    }
    for (i = initial_num_pages; i != page_count; i++) {
        page_mapped[i] = 0;
        page_avail[i] = i == page_count - 1 ? -1 : i + 1;
    }
    // initialize inverted page table
    int frame_count = 1<<FRAME_BITS;
    int inverted_page_table[frame_count];
    for (i = 0; i != frame_count; i++) {
        inverted_page_table[i] = -1;
    }
    // initialize next victim
    int next_victim = 0;

    while (1) {
        siginfo_t info;
        int sig = sigwaitinfo(&signals, &info);

        if (sig == SIGUSR1) {
            // retrieve the index of the page that the user program wants, or -1 if the user program has terminated
            int const requested_page = info.si_value.sival_int;

            if (requested_page == -1) break;

            // seg fault
            if (requested_page >= page_count || requested_page < 0 || !page_mapped[requested_page]) {
                union sigval reply_value;
                reply_value.sival_int = 1;
                sigqueue(info.si_pid, SIGCONT, reply_value);
                continue;
            }

            // CLOCK
            while (inverted_page_table[next_victim] != -1 && pg_table->entries[inverted_page_table[next_victim]].referenced) {
                pg_table->entries[inverted_page_table[next_victim]].referenced = 0;
                next_victim = (next_victim + 1) % frame_count;
            }
            if (inverted_page_table[next_victim] != -1) {
                // remove page from ram
                if (pg_table->entries[inverted_page_table[next_victim]].dirty) {
                    disk_write(next_victim, inverted_page_table[next_victim]);
                    pg_table->entries[inverted_page_table[next_victim]].dirty = 0;
                }
                pg_table->entries[inverted_page_table[next_victim]].valid = 0;
            }
            disk_read(next_victim, requested_page);
            pg_table->entries[requested_page].valid = 1;
            pg_table->entries[requested_page].dirty = 0;
            pg_table->entries[requested_page].frame_index = next_victim;
            inverted_page_table[next_victim] = requested_page;

            next_victim = (next_victim + 1) % frame_count;

            // tell the MMU that we are done updating the page table
            union sigval reply_value;
            // set to 0 if the page is successfully loaded, set to 1 if the page is not mapped to the user process (i.e. segfault)
            reply_value.sival_int = 0;
            sigqueue(info.si_pid, SIGCONT, reply_value);
        } else {
            int const signal = info.si_value.sival_int;

            // mmap
            if (signal == -1) {
                // label page as mapped
                page_mapped[cur_avail] = 1;
                // create page in disk
                disk_create(cur_avail);
                union sigval reply_value;
                reply_value.sival_int = cur_avail;
                // label page as unavailable
                cur_avail = page_avail[cur_avail];
                sigqueue(info.si_pid, SIGCONT, reply_value);
                continue;
            }

            // munmap
            if (signal >= 0 && signal < page_count && page_mapped[signal]) {
                // label page as unmapped
                page_mapped[signal] = 0;
                if (pg_table->entries[signal].valid) {
                    // remove page from ram
                    pg_table->entries[signal].valid = 0;
                    pg_table->entries[signal].dirty = 0;
                    inverted_page_table[pg_table->entries[signal].frame_index] = -1;
                }
                // remove page from disk
                disk_delete(signal);
                // label page as available
                page_avail[signal] = cur_avail;
                cur_avail = signal;
            }
            union sigval reply_value;
            // value does not matter
            reply_value.sival_int = 0;
            sigqueue(info.si_pid, SIGCONT, reply_value);
        }
    }
}
