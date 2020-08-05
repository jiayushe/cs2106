#ifndef API_H
#define API_H

#include "page_table.h"

// Any changes to this file will be discarded for grading.


/* You are to implement this function */

/**
 * For ex1+.  Simulates the OS page allocator for a user process.  Number of frames is a compile-time constant defined in page_table.h.
 * initial_num_pages:  The number of pages required by this process initially, numbered from 0 to (initial_num_pages-1).
 * pg_table:  The page_table struct that is shared with the MMU.  The valid bit of every page is guaranteed to be 0.
 */
void os_run(int initial_num_pages, page_table *pg_table);



/* You can call these functions that are already implemented in runner.c */

/**
 * For ex1+.  Asks the disk manager to read the specified page from disk into the given frame.  This operation takes some time to complete.
 * frame_num:  The frame in RAM to write the page to (0 to total_frames-1)
 * page_num:  The page that we want to load from disk (0 to total_pages-1)
 * If invalid parameters are provided or overwriting a dirty page is attempted, an error message will be printed and the program terminated.
 * This function is not reentrant, and also should not be simultaneously called with the other API functions.
 */
void disk_read(int frame_num, int page_num);

/**
 * For ex2+.  Asks the disk manager to write the specified page from the given frame to the disk.  This operation takes some time to complete.
 * frame_num:  The frame in RAM to read the page from (0 to total_frames-1)
 * page_num:  The page that we want to write to disk (0 to total_pages-1)
 * The page should already exist in the disk.  To create a new page on the disk, use disk_create().
 * If invalid parameters are provided or if the specified frame does not contain the specified page, an error message will be printed and the program terminated.
 * This function is not reentrant, and also should not be simultaneously called with the other API functions.
 */
void disk_write(int frame_num, int page_num);

/**
 * For ex1+.  Prepare and initialize a new page on the disk
 * page_num:  The page that we want to create on the disk (0 to total_pages-1) (it must not already exist on disk)
 * This function is not reentrant, and also should not be simultaneously called with the other API functions.
 */
void disk_create(int page_num);

/**
 * For ex3+.  Delete the page from the disk
 * page_num:  The page that we want to delete from disk (it must already exist on disk)
 * This function is not reentrant, and also should not be simultaneously called with the other API functions.
 */
void disk_delete(int page_num);

#endif
