// This file is part of RFUS (Rich's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

#pragma once

#include <rfus/task.hpp>
#include <rfus/rfus_type.hpp>

namespace rfus {

/**
* Interface for Rcythr's Fast Userspace Scheduling.
*   Implements functions which allow posting of task_t* objects for scheduling.
*/
struct RFUSInterface
{
    virtual ~RFUSInterface() {}

    /**
    * Schedules the given task's task_t* sub-object
    * This function is thread safe.
    * 
    * @param t the task to schedule.
    */
    template<typename TaskType>
    RFUSInterface* post(TaskType&& t)
    {
        post(t.close());
        return this;
    }

    /**
    * Schedules a properly built task_t* object directly.
    * This function is thread safe.
    *
    * @param t the task_t* to schedule.
    */
    virtual RFUSInterface* post(task_t* t) = 0;

    /**
    * Sets a function to be posted when an exception is caught by the RFUS.
    * This function is not thread safe.
    *
    * @arg handler the function to post.
    * @arg worker the worker which should handle the exception.
    * @arg priority the priority of the worker handling the exception.
    */
    virtual RFUSInterface* set_exception_handler(std::function<void(std::exception_ptr)> handler, size_t worker=0, uint64_t priority=0) = 0; 
};

/**
* Creates a new RFUS of the given type. (See rfus_type.h for more info on options)
* @param type the type of the RFUS to create. This cooresponds to how resource-less events are scheduled.
* @param num_threads the number of workers to create. This affects the number of independent work queues.
*                       if the number of resources > num_threads some resources will share a thread.
* @param step_size the maximum number of events each worker removes from the queue at a time. 0 is infinite.
*/
RFUSInterface* createRFUS(RFUSType type, size_t num_threads, size_t step_size);

/**
* Global pointer for use as a singleton if desired.
*/
extern RFUSInterface* RFUS;

}
