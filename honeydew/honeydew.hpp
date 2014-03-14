// This file is part of Honeydew
// Honeydew is licensed under the MIT LICENSE. See the LICENSE file for more info.

#pragma once

#include <honeydew/task_t.hpp>

namespace honeydew {

/**
* Interface for Honeydew implementing classes.
*   Implements functions which allow posting of task_t* objects for scheduling.
*/
struct Honeydew
{
    enum HoneydewType
    {
        ROUND_ROBIN,
        ROUND_ROBIN_WITH_PRIORITY,
        LEAST_BUSY,
        LEAST_BUSY_WITH_PRIORITY
    };

    virtual ~Honeydew() {}

    /**
    * Creates a new Honeydew of the given type. (See honeydew_type.h for more info on options)
    * @param type the type of the Honeydew to create. This cooresponds to how resource-less events are scheduled.
    * @param num_threads the number of workers to create. This affects the number of independent work queues.
    *                       if the number of resources > num_threads some resources will share a thread.
    * @param step_size the maximum number of events each worker removes from the queue at a time. 0 is infinite.
    */
    static Honeydew* create(HoneydewType type, size_t num_threads, size_t step_size);

    /**
    * Schedules the given task's task_t* sub-object
    * This function is thread safe.
    * 
    * @param t the task to schedule.
    */
    template<typename TaskType>
    Honeydew* post(TaskType&& t)
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
    virtual Honeydew* post(task_t* t) = 0;

    /**
    * Sets a function to be posted when an exception is caught by the Honeydew.
    * This function is not thread safe.
    *
    * @arg handler the function to post.
    * @arg worker the worker which should handle the exception.
    * @arg priority the priority of the worker handling the exception.
    */
    virtual Honeydew* set_exception_handler(std::function<void(std::exception_ptr)> handler, size_t worker=0, uint64_t priority=0) = 0; 
};

}
