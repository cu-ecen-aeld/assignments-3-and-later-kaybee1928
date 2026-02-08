#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    struct timespec ots;
    struct timespec rts;
    ots.tv_sec = thread_func_args->wait_to_obtain_ms / 1000.0;
    ots.tv_nsec = (thread_func_args->wait_to_obtain_ms % 1000) * 1000000;
    rts.tv_sec = thread_func_args->wait_to_release_ms / 1000.0;
    rts.tv_nsec = (thread_func_args->wait_to_release_ms % 1000) * 1000000;
    nanosleep(&ots, NULL);
    int rc = pthread_mutex_lock(thread_func_args->mutex);
    if (rc != 0) {
        thread_func_args->thread_complete_success = false;
        return thread_func_args;
    }
    nanosleep(&rts, NULL);
    rc = pthread_mutex_unlock(thread_func_args->mutex);
    if (rc != 0) {
        thread_func_args->thread_complete_success = false;
    }
    return thread_func_args;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    struct thread_data* thread_func_args = malloc(sizeof(struct thread_data));
    thread_func_args->thread_id = thread;
    thread_func_args->mutex = mutex;
    thread_func_args->wait_to_obtain_ms = wait_to_obtain_ms;
    thread_func_args->wait_to_release_ms = wait_to_release_ms;
    thread_func_args->thread_complete_success = true;
    // Looks like it was already initialized in test. oooooooofffffffffffffff
    // pthread_mutex_init(mutex, NULL);
    int rc = pthread_create(thread, NULL, threadfunc, thread_func_args);
    if (rc == 0) {
        return true;
    }
    free(thread_func_args);
    return false;
}

