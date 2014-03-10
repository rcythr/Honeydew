// Rcythr's Fast Userspace Scheduling
#pragma once

#include "task.h"
#include "rfus_type.h"

#define ENABLE_SINGLETON 1

/**
* Interface for Rcythr's Fast Userspace Scheduling.
*   Implements functions which allow posting of task_t* objects for scheduling.
*/
struct RFUSInterface
{
    virtual ~RFUSInterface() {}

    /**
    * Schedules the given task's task_t* sub-object
    * @param t the task to schedule.
    */
    virtual RFUSInterface* post(Task& t) = 0;

    /**
    * Schedules a properly built task_t* object directly.
    * @param t the task_t* to schedule.
    */
    virtual RFUSInterface* post(task_t* t) = 0;
};

/**
* Creates a new RFUS of the given type. (See rfus_type.h for more info on options)
* @param type the type of the RFUS to create. This cooresponds to how resource-less events are scheduled.
* @param num_threads the number of workers to create. This affects the number of independent work queues.
*                       if the number of resources > num_threads some resources will share a thread.
* @param step_size the maximum number of events each worker removes from the queue at a time. 0 is infinite.
*/
RFUSInterface* createRFUS(RFUSType type, size_t num_threads, size_t step_size);

#if ENABLE_SINGLETON
    /**
    * Global pointer for use as a singleton if desired.
    */
    extern RFUSInterface* RFUS;
#endif
