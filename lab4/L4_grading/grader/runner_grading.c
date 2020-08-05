// Grader file that will be compiled with your solution.
// Any changes to this file will be discarded for grading.

#include <ctype.h>
#include <inttypes.h>
#include <errno.h>
#include <signal.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <pthread.h>
#include <semaphore.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#include "api.h"

#include "fast_framedata.h"

#define BILLION 1000000000

// input format:
// first line: <initial_num_pages>
// subsequent lines:
// r <pagenum> // read from the given page
// w <pagenum> // write to the given page
// m <pagenum> // mmap a new page (pagenum is used to refer to this page in the input only, the OS might give us some other page instead)
// u <pagenum> // munmap an existing page


// For grading, the input file identifier number is printed instead of the OS-assigned page number
// #defines for grading:
// Use efficiency constraints:
// #define GRADING_EFFICIENCY
// Print disk_read() calls
// #define GRADING_READ
// Print disk_read() calls
// #define GRADING_WRITE
// Print disk_write() calls and warnings
// #define GRADING_MMAP_MUNMAP
// Print mmap and munmap results
// #define GRADING_DELETE
// Print disk_delete() calls, except for cleanup on termination
// #define GRADING_CREATE
// Print disk_create() calls



static int sigwaitinfo_with_nanos(const sigset_t *set, siginfo_t *info, uint64_t time_remaining) {
    struct timespec timeout;
    timeout.tv_sec = time_remaining / BILLION;
    timeout.tv_nsec = time_remaining % BILLION;
    return sigtimedwait(set, info, &timeout);
}

pid_t childpid = -1;
static void os_timeout() {
    printf("OS timeout\n");
    kill(childpid, SIGKILL);
    exit(0);
}
static void os_fast_memcorrupted() {
    printf("OS corrupted the memory of user process (in fast grading mode)\n");
    kill(childpid, SIGKILL);
    exit(0);
}


typedef struct {
    unsigned page_index : PAGE_BITS; // the page stored in this frame
    unsigned valid : 1; // whether this frame is valid (if not valid, then page_index and dirty contains garbage)
    unsigned dirty : 1; // whether this frame is dirty
    os_list_node node_data;
} frame_entry;

#define DISK_READ 0
#define DISK_WRITE 1
#define DISK_CREATE 2
#define DISK_DELETE 3
#define DISK_TERMINATE 4

typedef struct {
    sem_t os_to_runner_sem;
    sem_t runner_to_os_sem;
    int type; // 0=disk_read, 1=disk_write, 2=terminate the listener thread
    int frame_num;
    int page_num;
    int kill; // for runner to kill the OS process
    
    int init_num_pages; // not really disk data, but we need to communicate to the other process
    pthread_barrier_t init_barrier;
    
    int silence_delete;
} disk_comm_data;

typedef struct {
    frame_entry *framedata;
    int *page_dirty;
    int *page_exists_on_disk;
    int *pagemapinv;
    int *page_used;
} disk_thread_params;

static disk_comm_data * disk_comm;

static void init_disk_comm(void) {
    sem_init(&disk_comm->os_to_runner_sem, 1, 0);
    sem_init(&disk_comm->runner_to_os_sem, 1, 0);
    disk_comm->kill = 0;
    pthread_barrierattr_t init_barrier_attr;
    pthread_barrierattr_init(&init_barrier_attr);
    pthread_barrierattr_setpshared(&init_barrier_attr, PTHREAD_PROCESS_SHARED);
    pthread_barrier_init(&disk_comm->init_barrier, &init_barrier_attr, 3);
    pthread_barrierattr_destroy(&init_barrier_attr);
    disk_comm->silence_delete = 0;
}

void disk_read(int frame_num, int page_num) {
    disk_comm->type = DISK_READ;
    disk_comm->frame_num = frame_num;
    disk_comm->page_num = page_num;
    sem_post(&disk_comm->os_to_runner_sem);
    sem_wait(&disk_comm->runner_to_os_sem);
    if (disk_comm->kill) exit(0);
}

void disk_write(int frame_num, int page_num) {
    disk_comm->type = DISK_WRITE;
    disk_comm->frame_num = frame_num;
    disk_comm->page_num = page_num;
    sem_post(&disk_comm->os_to_runner_sem);
    sem_wait(&disk_comm->runner_to_os_sem);
    if (disk_comm->kill) exit(0);
}

void disk_create(int page_num) {
    fflush(stdout);
    disk_comm->type = DISK_CREATE;
    disk_comm->page_num = page_num;
    sem_post(&disk_comm->os_to_runner_sem);
    sem_wait(&disk_comm->runner_to_os_sem);
    if (disk_comm->kill) exit(0);
}

void disk_delete(int page_num) {
    fflush(stdout);
    disk_comm->type = DISK_DELETE;
    disk_comm->page_num = page_num;
    sem_post(&disk_comm->os_to_runner_sem);
    sem_wait(&disk_comm->runner_to_os_sem);
    if (disk_comm->kill) exit(0);
}

static void destroy_disk_comm(void) {
    pthread_barrier_destroy(&disk_comm->init_barrier);
    sem_destroy(&disk_comm->runner_to_os_sem);
    sem_destroy(&disk_comm->os_to_runner_sem);
}

static void setsigmask() {
    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGCONT);
    sigprocmask(SIG_BLOCK, &signals, NULL);
}

static void *disk_listen(void *param) {
    // set up the signal blocked mask for the disk thread
    setsigmask();
    
    // start the OS
    pthread_barrier_wait(&disk_comm->init_barrier);
    
    disk_thread_params const * const disk_params = param;
    frame_entry * const framedata = disk_params->framedata;
    int * const page_dirty = disk_params->page_dirty;
    int * const page_exists_on_disk = disk_params->page_exists_on_disk;
    int * const pagemapinv = disk_params->pagemapinv;
    int * const page_used = disk_params->page_used;
    while(1) {
        sem_wait(&disk_comm->os_to_runner_sem);
        int killed = 0;
        if (disk_comm->type == DISK_TERMINATE) break;
        else if (disk_comm->type == DISK_READ) {
            // read
            if (disk_comm->page_num < 0 || disk_comm->page_num >= (1<<PAGE_BITS) || !page_exists_on_disk[disk_comm->page_num]) {
                printf("disk_read(): OS tried to load page %d, but it is not on the disk!\n", disk_comm->page_num);
                killed = 1;
            }
            else if (disk_comm->frame_num < 0 || disk_comm->frame_num >= (1<<FRAME_BITS)) {
                printf("disk_read(): OS tried to load a page into frame %d, which is an invalid frame index!\n", disk_comm->frame_num);
                killed = 1;
            }
            else if (framedata[disk_comm->frame_num].valid && framedata[disk_comm->frame_num].dirty) {
                printf("disk_read(): OS tried to overwrite dirty page %d without saving it to disk first!\n", framedata[disk_comm->frame_num].page_index);
                killed = 1;
            }
            else {
                if (framedata[disk_comm->frame_num].valid) {
                    os_deregister_list_node(&framedata[disk_comm->frame_num].node_data);
                }
                framedata[disk_comm->frame_num].page_index = disk_comm->page_num;
                framedata[disk_comm->frame_num].valid = 1;
                framedata[disk_comm->frame_num].dirty = 0;
                os_register_list_node(&framedata[disk_comm->frame_num].node_data, disk_comm->page_num);
#ifdef GRADING_READ
                printf("disk_read(): Loading frame %d from disk page %d... OK.\n", disk_comm->frame_num, pagemapinv[disk_comm->page_num]);
#endif
            }
        }
        else if (disk_comm->type == DISK_WRITE) {
            // write
            if (disk_comm->page_num < 0 || disk_comm->page_num >= (1<<PAGE_BITS)) {
                printf("disk_write(): OS tried to write invalid page %d to the disk!\n", disk_comm->page_num);
                killed = 1;
            }
            else if (disk_comm->frame_num < 0 || disk_comm->frame_num >= (1<<FRAME_BITS) || !framedata[disk_comm->frame_num].valid) {
                printf("disk_write(): OS tried to write invalid frame %d to the disk!\n", disk_comm->frame_num);
                killed = 1;
            }
            else if (framedata[disk_comm->frame_num].page_index != disk_comm->page_num || framedata[disk_comm->frame_num].dirty != page_dirty[disk_comm->page_num]) {
                printf("disk_write(): OS tried to write frame %d to the wrong page (page %d) on the disk!\n", disk_comm->frame_num, disk_comm->page_num);
                killed = 1;
            }
            else {
#ifdef GRADING_WRITE
                if (!framedata[disk_comm->frame_num].dirty) {
                    printf("disk_write(): Warning, OS is writing non-dirty page %d to disk.\n", disk_comm->page_num);
                }
#endif
                framedata[disk_comm->frame_num].dirty = 0;
                page_dirty[disk_comm->page_num] = 0;
                page_exists_on_disk[disk_comm->page_num] = 1;
#ifdef GRADING_WRITE
                printf("disk_write(): Writing frame %d to disk page %d... OK.\n", disk_comm->frame_num, pagemapinv[disk_comm->page_num]);
#endif
            }
        }
        else if (disk_comm->type == DISK_CREATE) {
            // create page
            if (disk_comm->page_num < 0 || disk_comm->page_num >= (1<<PAGE_BITS)) {
                printf("disk_create(): OS tried to create invalid page %d on the disk!\n", disk_comm->page_num);
                killed = 1;
            }
            else if (page_exists_on_disk[disk_comm->page_num]) {
                printf("disk_create(): OS tried to create page %d, which already exists in disk!\n", disk_comm->page_num);
                killed = 1;
            }
            else {
                page_exists_on_disk[disk_comm->page_num] = 1;
#ifdef GRADING_CREATE
                printf("disk_create(): Creating disk page %d... OK.\n", pagemapinv[disk_comm->page_num]);
#endif
            }
        }
        else if (disk_comm->type == DISK_DELETE) {
            // delete page
            if (disk_comm->page_num < 0 || disk_comm->page_num >= (1<<PAGE_BITS)) {
                printf("disk_delete(): OS tried to delete page %d, which is an invalid page, from the disk!\n", disk_comm->page_num);
                killed = 1;
            }
            else if (!page_exists_on_disk[disk_comm->page_num]) {
                printf("disk_delete(): OS tried to delete page %d, which does not already exist on disk!\n", disk_comm->page_num);
                killed = 1;
            }
            else {
                page_exists_on_disk[disk_comm->page_num] = 0;
#ifdef GRADING_DELETE
                // Note: In older runner, disk_delete() will freeze up if called after OS received -1 (termination signal)
                if (!disk_comm->silence_delete) {
#ifdef GRADING_CREATE
                    printf("disk_delete(): Deleting disk page %d... OK.\n", pagemapinv[disk_comm->page_num]);
#else
                    if (page_used[disk_comm->page_num] ) { // so that an ex4 solution might not delete an unused page
                        printf("disk_delete(): Deleting disk page %d... OK.\n", pagemapinv[disk_comm->page_num]);
                        page_used[disk_comm->page_num] = 0;
                    }
#endif
                }
#endif
            }
        }
        disk_comm->kill = killed;
        sem_post(&disk_comm->runner_to_os_sem);
        if (killed) break;
    }
    return NULL;
}


// check if all the entries in the page table still look correct
// returns 0 if okay
// returns -1 if there are errors
static int validate_page_table(page_table const *pg_table, frame_entry const *frame_data, int const *page_dirty) {
#ifndef GRADING_EFFICIENCY
    for (int i = 0; i != (1<<PAGE_BITS); ++i) {
        page_table_entry const * const entry = pg_table->entries + i;
        if (entry->valid) {
            if (entry->frame_index < 0 || entry->frame_index >= (1<<FRAME_BITS)) {
                return -1; // page table contains invalid frame_index
            }
            frame_entry const * const frame_ptr = frame_data + entry->frame_index;
            if (!frame_ptr->valid) {
                return -1; // user process can now access frames containing invalid data
            }
            if (frame_ptr->page_index != i) {
                return -1; // user process can now access frames containing data for the wrong page
            }
            if (frame_ptr->dirty != page_dirty[i]) {
                return -1; // the frame containing this page is stale (maybe the OS loaded the same page into multiple frames)
            }
        }
    }
#endif
    return 0;
}


// returns the new page number assigned by the OS, or -1 if failed
static int exec_mmap_and_check_result(page_table *pg_table, frame_entry const *frame_data, int const *page_dirty, int *page_mapped, pid_t child_pid, uint64_t *time_remaining) {
    
    atomic_thread_fence(memory_order_release);
    
    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    union sigval reply_value;
    reply_value.sival_int = -1; // -1 for mmap, any nonnegative integer is the page to munmap
    sigqueue(child_pid, SIGUSR2, reply_value);
    
    // wait for OS to tell us that it is done loading
    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGCONT);
    siginfo_t info;
    sigwaitinfo_with_nanos(&signals, &info, *time_remaining);
    
    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    uint64_t time_spent = ((uint64_t)end_time.tv_sec * BILLION + (uint64_t)end_time.tv_nsec) - ((uint64_t)start_time.tv_sec * BILLION + (uint64_t)start_time.tv_nsec);
    if (time_spent > *time_remaining) os_timeout();
    *time_remaining -= time_spent;
    
    // // not sure if sigwaitinfo will synchronize memory... so we explicitly do it
    atomic_thread_fence(memory_order_acquire);
    int const new_page_num = info.si_value.sival_int;
    
    if (new_page_num < 0 || new_page_num >= (1<<PAGE_BITS)) {
        printf("Runner: OS returned invalid page number %d when mapping new page!\n", new_page_num);
        return -1;
    }
    if (page_mapped[new_page_num]) {
        printf("Runner: OS returned already-mapped page number %d when mapping new page!\n", new_page_num);
        return -1;
    }
    page_mapped[new_page_num] = 1;
    
    if (validate_page_table(pg_table, frame_data, page_dirty)) {
        printf("Runner: Memory of user process is corrupted; OS has a bug!\n");
        return -1;
    }
    
    return new_page_num;
}

// returns -1 if failed, 0 if succeeded
// munmap will succeed (and do nothing) if page is already unmapped
static int exec_munmap_and_check_result(page_table *pg_table, frame_entry *frame_data, int *page_dirty, int const *page_exists_on_disk, int pagenum, int *page_mapped, pid_t child_pid, uint64_t *time_remaining) {
    
    atomic_thread_fence(memory_order_release);
    
    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    union sigval reply_value;
    reply_value.sival_int = pagenum; // -1 for mmap, any nonnegative integer is the page to munmap
    sigqueue(child_pid, SIGUSR2, reply_value);
    
    // wait for OS to tell us that it is done loading
    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGCONT);
    siginfo_t info;
    sigwaitinfo_with_nanos(&signals, &info, *time_remaining);
    
    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    uint64_t time_spent = ((uint64_t)end_time.tv_sec * BILLION + (uint64_t)end_time.tv_nsec) - ((uint64_t)start_time.tv_sec * BILLION + (uint64_t)start_time.tv_nsec);
    if (time_spent > *time_remaining) os_timeout();
    *time_remaining -= time_spent;
    
    // // not sure if sigwaitinfo will synchronize memory... so we explicitly do it
    atomic_thread_fence(memory_order_acquire);
    
    page_mapped[pagenum] = 0;
    page_dirty[pagenum] = 0;
    
    for (os_list_node const* it = os_get_list(pagenum); it; it = os_list_next(pagenum, it)) {
        frame_entry* entry = (frame_entry*)(((char*)it) - offsetof(frame_entry, node_data));
        if (!entry->valid || entry->page_index != pagenum) {
            os_fast_memcorrupted();
        }
        entry->valid = 0;
    }
    os_clear_list(pagenum);
    
    if (validate_page_table(pg_table, frame_data, page_dirty)) {
        printf("Runner: Memory of user process is corrupted; OS has a bug!\n");
        return -1;
    }
    if (page_exists_on_disk[pagenum]) {
        printf("Runner: OS did not delete page %d from disk after munmap!\n", pagenum);
        return -1;
    }
    
    return 0;
}


// returns 0 for success
// returns -1 for OS not loading the page properly
// returns -2 for user process segfault
static int ensure_page_loaded(page_table *pg_table, frame_entry const *frame_data, int const *page_dirty, int page_index, pid_t child_pid, int expects_segfault, uint64_t *time_remaining) {
    page_table_entry const * const entry = pg_table->entries + page_index;
    
    // if the page is not in RAM (i.e. not valid), ask the OS to load the page
    if (!entry->valid) {
        atomic_thread_fence(memory_order_release);
    
        struct timespec start_time;
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        
        union sigval reply_value;
        reply_value.sival_int = page_index; // tells the OS process to load the given page_index
        sigqueue(child_pid, SIGUSR1, reply_value);
        
        // wait for OS to tell us that it is done loading
        sigset_t signals;
        sigemptyset(&signals);
        sigaddset(&signals, SIGCONT);
        siginfo_t info;
        sigwaitinfo_with_nanos(&signals, &info, *time_remaining);
        
        struct timespec end_time;
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        uint64_t time_spent = ((uint64_t)end_time.tv_sec * BILLION + (uint64_t)end_time.tv_nsec) - ((uint64_t)start_time.tv_sec * BILLION + (uint64_t)start_time.tv_nsec);
        if (time_spent > *time_remaining) os_timeout();
        *time_remaining -= time_spent;
        // // not sure if sigwaitinfo will synchronize memory... so we explicitly do it
        atomic_thread_fence(memory_order_acquire);
        int const is_segfault = info.si_value.sival_int;
        if (!expects_segfault) {
            if (is_segfault) return -2;
            
            // if the page is still not in RAM, then the OS has a bug
            if (!entry->valid) {
                return -1;
            }
            
            // check that the page table looks correct
            if (validate_page_table(pg_table, frame_data, page_dirty) != 0) return -1;
        }
        else {
            if (!is_segfault) return 0;
            
            // check that the page table looks correct
            if (validate_page_table(pg_table, frame_data, page_dirty) != 0) return -1;
            
            return -2;
        }
    }
    
    // set the referenced bit, so that the OS can use this information in its page replacement algorithm
    pg_table->entries[page_index].referenced = 1;
    
    return 0;
}

static void set_dirty_flag(page_table *pg_table, frame_entry *frame_data, int *page_dirty, int page_index) {
    // update our internal trackers, so we can figure out if the OS is doing things correctly
    page_dirty[page_index] = 1;
    page_table_entry * const entry = pg_table->entries + page_index;
#ifdef GRADING_EFFICIENCY
    if (!entry->valid || entry->frame_index >= (1<<FRAME_BITS)) os_fast_memcorrupted(); // necessary because  GRADING_EFFICIENCY disables validate_page_table()
#endif
    frame_data[entry->frame_index].dirty = 1;
    // set the dirty bit, so that the OS will write the page back to disk if it gets replaced
    entry->dirty = 1;
}

int main() {
    
    page_table *pg_table = (void*)mmap(NULL, sizeof(page_table), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    disk_comm = (void*)mmap(NULL, sizeof(disk_comm_data), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    // mmap with MAP_ANONYMOUS zeroes the memory automatically
    // note: Solaris mmap() returns char* instead of void*
    
    if (pg_table == MAP_FAILED || disk_comm == MAP_FAILED) {
        printf("Runner: Failed to allocate shared memory!\n");
        return 0;
    }
    
    init_disk_comm();
    
    int child_pid = fork();
    if (child_pid == 0) {
        // we are the child (OS process)
        
        sem_wait(&disk_comm->runner_to_os_sem);
        if (disk_comm->init_num_pages <= 0) return 0;
        close(STDIN_FILENO);
        
        // silence stdout and stderr from the OS
        int null_fd = open("/dev/null", O_WRONLY | O_CREAT, 0666);
        dup2(null_fd, STDOUT_FILENO);
        dup2(null_fd, STDERR_FILENO);
        
        sigset_t signals;
        sigemptyset(&signals);
        sigaddset(&signals, SIGUSR1);
        sigaddset(&signals, SIGUSR2);
        sigprocmask(SIG_BLOCK, &signals, NULL);
        pthread_barrier_wait(&disk_comm->init_barrier);
        
        os_run(disk_comm->init_num_pages, pg_table);
        
        close(null_fd);
    }
    else {
        childpid = child_pid;
        sigset_t sigchld;
        sigemptyset(&sigchld);
        sigaddset(&sigchld, SIGCHLD);
        sigprocmask(SIG_BLOCK, &sigchld, NULL);
        // read the initial number of pages
        int init_num_pages;
        scanf("%d", &init_num_pages);
        if (init_num_pages < 0) {
            printf("Runner: Need non-negative number of pages!\n");
            init_num_pages = 0;
        }
        if (init_num_pages >= (1 << PAGE_BITS)) {
            printf("Runner: Too many pages!\n");
            init_num_pages = 0;
        }
        
        disk_comm->init_num_pages = init_num_pages;
        sem_post(&disk_comm->runner_to_os_sem);
        
        if (init_num_pages > 0) {
            
            os_list_init();
        
            // translation from input page index to OS's page index
            int *pagemap = malloc(sizeof(int) * (1 << PAGE_BITS));
            int *pagemapinv = malloc(sizeof(int) * (1 << PAGE_BITS));
            // stores an inverse page table (from frame index to OS's page index)
            frame_entry *framedata = malloc(sizeof(frame_entry) * (1 << FRAME_BITS));
            // stores whether each page (in terms of OS's page index) is actually dirty (in case the OS loads the same page into multiple frames)
            int *page_dirty = malloc(sizeof(int) * (1 << PAGE_BITS));
            int *page_exists_on_disk = malloc(sizeof(int) * (1 << PAGE_BITS));
            int *page_mapped = malloc(sizeof(int) * (1 << PAGE_BITS));
            int *page_used = malloc(sizeof(int) * (1 << PAGE_BITS));
            
            for (int i = 0; i != init_num_pages; ++i) {
                page_mapped[i] = 1;
            }
            for (int i = init_num_pages; i != (1 << PAGE_BITS); ++i) {
                page_mapped[i] = 0;
            }
            for (int i = 0; i != (1 << FRAME_BITS); ++i) {
                framedata[i].valid = 0;
            }
            for (int i = 0; i != (1 << PAGE_BITS); ++i) {
                pagemap[i] = i; // we start off with the identity permutation
                pagemapinv[i] = i; // we start off with the identity permutation
                page_dirty[i] = 0;
                page_exists_on_disk[i] = 0;
                page_used[i] = 0;
            }
            
            // set up the signal blocked mask for the main thread
            setsigmask();
            
            pthread_t disk_thread;
            disk_thread_params disk_params = { .framedata = framedata, .page_dirty = page_dirty, .page_exists_on_disk = page_exists_on_disk, .pagemapinv = pagemapinv, .page_used = page_used };
            pthread_create(&disk_thread, NULL, &disk_listen, &disk_params);
            
            // wait until the disk thread and the OS process are done with initialisation
            pthread_barrier_wait(&disk_comm->init_barrier);
            
#ifdef GRADING_EFFICIENCY
            uint64_t time_remaining = 4000000000ull; // 4 seconds (it should take <1s in practice)
#else
            uint64_t time_remaining = 0;
#endif
            
            char instruction;
            int counter = 0;
            while (scanf(" %c", &instruction) == 1) {
                ++counter;
#ifdef GRADING_EFFICIENCY
                time_remaining += 1000000; // 1 millisecond
#else
                time_remaining = BILLION; // 1 second
#endif
                int is_unmapped_page;
                {
                    int ch;
                    do {
                        ch = getc(stdin);
                    } while (isspace(ch));
                    if (ch == EOF) {
                        printf("Runner: Input file has invalid format!\n");
                        break;
                    }
                    if (ch == '*') {
                        is_unmapped_page = 1;
                    }
                    else {
                        is_unmapped_page = 0;
                        ungetc(ch, stdin);
                    }
                }
                int pagenum;
                if (scanf("%d", &pagenum) != 1) {
                    printf("Runner: Input file has invalid format!\n");
                    break;
                }
                if (pagenum < 0 || pagenum >= (1 << PAGE_BITS)) {
                    printf("Runner: Input file has invalid (out of bounds) page number %d!\n", pagenum);
                    break;
                }
                
                // echo the input, so that we can tell that a disk API call comes from the correct command
                if (is_unmapped_page) printf("Input: %c*%d\n", instruction, pagenum);
                else printf("Input: %c%d\n", instruction, pagenum);
                
                if (instruction == 'r' || instruction == 'w') {
                    if (!is_unmapped_page) {
                        if (!page_mapped[pagemap[pagenum]]) {
                            printf("Runner: Input file has unmapped page number (index in input file = %d, index sent to OS = %d) for read or write!\n", pagenum, pagemap[pagenum]);
                            break;
                        }
                        page_used[pagemap[pagenum]] = 1;
                        int res = ensure_page_loaded(pg_table, framedata, page_dirty, pagemap[pagenum], child_pid, is_unmapped_page, &time_remaining);
                        if (res == -1) {
                            printf("Runner: Memory of user process is corrupted; OS has a bug!\n");
                            break;
                        }
                        if (res == -2) {
                            printf("Runner: OS reports that user process tried to read from an invalid page (segfault), but this isn't the case!\n");
                            break;
                        }
                        if (instruction == 'w') {
                            // write
                            set_dirty_flag(pg_table, framedata, page_dirty, pagemap[pagenum]);
                        }
                    }
                    else {
                        if (page_mapped[pagemap[pagenum]]) {
                            printf("Runner: Input file expects an unmapped page number for read or write to unmapped memory location, but got a mapped page (index in input file = %d, index sent to OS = %d)!\n", pagenum, pagemap[pagenum]);
                            break;
                        }
                        int res = ensure_page_loaded(pg_table, framedata, page_dirty, pagemap[pagenum], child_pid, is_unmapped_page, &time_remaining);
                        if (res == 0) {
                            printf("Runner: User process tried to request for unmapped page %d, but OS did not report the segfault!\n", pagemap[pagenum]);
                            break;
                        }
                        if (res == -1) {
                            printf("Runner: Memory of user process is corrupted; OS has a bug!\n");
                            break;
                        }
                        if (res == -2) {
                            printf("Runner: OS detected user process segfault... OK.\n");
                        }
                    }
                }
                else if (instruction == 'm') {
                    // mmap
                    if (is_unmapped_page) {
                        printf("Runner: Input file has mmap *, this is not allowed!\n");
                        break;
                    }
                    if (page_mapped[pagemap[pagenum]]) {
                        printf("Runner: Input file used existing page number (index in input file = %d, index sent to OS = %d) for mmap!\n", pagenum, pagemap[pagenum]);
                        break;
                    }
                    int const realnewpage = exec_mmap_and_check_result(pg_table, framedata, page_dirty, page_mapped, child_pid, &time_remaining);
                    if (realnewpage == -1) break;
                    {
                        // add a transposition to the permutation
                        // this also works if pagemap[pagenum] == realnewpage
                        int const otherpage = pagemapinv[realnewpage];
                        pagemap[otherpage] = pagemap[pagenum];
                        pagemapinv[pagemap[pagenum]] = otherpage;
                        pagemap[pagenum] = realnewpage;
                        pagemapinv[realnewpage] = pagenum;
                    }
#ifdef GRADING_MMAP_MUNMAP
                    printf("Runner: mmap operation OK, OS mapped page %d.\n", pagenum);
#endif
                }
                else if (instruction == 'u') {
                    // unumap
                    if (!is_unmapped_page && !page_mapped[pagemap[pagenum]]) {
                        printf("Runner: Input file has nonexistent page number (index in input file = %d, index sent to OS = %d) for munmap!\n", pagenum, pagemap[pagenum]);
                        break;
                    }
                    if (is_unmapped_page && page_mapped[pagemap[pagenum]]) {
                        printf("Runner: Input file has expects nonexistent page number for munmap *, but got an existing page (index in input file = %d, index sent to OS = %d)!\n", pagenum, pagemap[pagenum]);
                        break;
                    }
                    int const res = exec_munmap_and_check_result(pg_table, framedata, page_dirty, page_exists_on_disk, pagemap[pagenum], page_mapped, child_pid, &time_remaining);
                    if (res == -1) break;
#ifdef GRADING_MMAP_MUNMAP
                    if (!is_unmapped_page) {
                        printf("Runner: munmap operation OK, OS unmapped page %d.\n", pagenum);
                    }
                    else {
                        printf("Runner: munmap operation OK, OS did not unmap any page because page %d was already unmapped.\n", pagenum);
                    }
#endif
                }
                else {
                    printf("Runner: Input file has invalid instruction type '%c'!\n", instruction);
                    break;
                }
            }
            if (ferror(stdin)) {
                printf("Runner: Input file has incorrect format!\n");
            }
            
            if (!disk_comm->kill) { // if the process wasn't killed due to invalid disk read/write
                // send SIGUSR1 signal to ask the OS process to terminate
                disk_comm->silence_delete = 1;
                union sigval reply_value;
                reply_value.sival_int = -1; // tells the OS process to terminate
                sigqueue(child_pid, SIGUSR1, reply_value);
            }
            
            {
#ifndef GRADING_EFFICIENCY
                time_remaining = BILLION; // 1 second
#endif
                sigwaitinfo_with_nanos(&sigchld, NULL, time_remaining);
                if (errno == EAGAIN) {
                    os_timeout();
                }
            }
            
            if (!disk_comm->kill) { // if the process wasn't killed due to invalid disk read/write
                // terminate the disk listener thread
                disk_comm->type = DISK_TERMINATE;
                sem_post(&disk_comm->os_to_runner_sem);
            }
            
            pthread_join(disk_thread, NULL);
            
            
            free(page_mapped);
            free(page_exists_on_disk);
            free(page_dirty);
            free(framedata);
            free(pagemapinv);
            free(pagemap);
        }
        
        destroy_disk_comm();
    }
    
    munmap((void*)disk_comm, sizeof(disk_comm_data));
    munmap((void*)pg_table, sizeof(page_table));
}
