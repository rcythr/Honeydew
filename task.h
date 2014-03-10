// This file is part of RFUS (Rcythr's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

#pragma once

#include <functional>
#include <cstdint>
#include <vector>

namespace rcythr
{

/// Forward Declarations
struct join_semaphore_t;

/**
* Structure used in the actual RFUS implementation.
*/
struct task_t
{
    task_t(std::function<void()> action, uint64_t deadline, size_t worker);

    // User vars
    std::function<void()> action;
    uint64_t priority;

    // Book keeping
    task_t* continuation;
    join_semaphore_t* join;
    size_t worker;

    task_t *next;
};

/**
* Class that allows for easy building of task_t* structures.
*  Usage is expected to be via daisy-chaining of function calls
*  onto this object like Task(func1).then(func2).also(func3);
*/
class Task
{
public:

    Task(Task&& other);
    Task(const Task& other) = delete;

    /**
    * Constructs a new task object with a given deadline.
    * @arg function to schedule and call on the associated worker thread.
    * @arg worker the associated thread on which to run this action. Worker=0 means any worker.
    * @arg deadline the absolute deadline (priority) of this call.
    */
    Task(std::function<void()> action, size_t worker=0, uint64_t deadline=0);

    /**
    * Cleans up the internals of this object if necessary.
    */
    ~Task();

    /**
    * Schedules a task with the given priority to be run after the previous task(s)
    *   on the given worker thread
    * @arg action the task to be performed.
    * @arg worker the associated worker for this task to run on. Worker=0 means any worker.
    * @arg deadline the priority of the task (added to the previous task's deadline).
    */
    Task& then(std::function<void()> action, size_t worker=0, uint64_t deadline=0);

    /**
    * Schedules a task with the given priority to be run after the previous task(s)
    *   on the given worker thread
    * @arg action the task to be performed.
    * @arg worker the associated worker for this task to run on. Worker=0 means any worker.
    * @arg deadline the priority of the task (absolute, not added to previous task's deadline).
    */
    Task& thenAbsolute(std::function<void()> action, size_t worker=0,  uint64_t deadline=0);

    /**
    * Schedules a task to occur concurrently with the previous task with the given priority
    *  on the associated worker thread. Further tasks will wait for this task to complete.
    * @arg action the task to perform.
    * @arg worker the associated worker thread. Worker=0 means any worker.
    * @arg deadline the priority of the task. (added to the previous task's deadline).
    */
    Task& also(std::function<void()> action, size_t worker=0, uint64_t deadline=0);

    /**
    * Schedules a task to occur concurrently with the previous task with the given priority
    *  on the associated worker thread. Further tasks will wait for this task to complete.
    * @arg action the task to perform.
    * @arg worker the associated worker thread. Worker=0 means any worker.
    * @arg deadline the absolute priority of the task. (not added to the previous task's deadline).
    */
    Task& alsoAbsolute(std::function<void()> action, size_t worker=0, uint64_t deadline=0);

    /**
    * Schedules a task to occur concurrently with the previous task with the given priority
    *  on the associated worker thread. Further tasks will not wait for this task to complete.
    * @arg action the task to perform.
    * @arg worker the associated worker thread. Worker=0 means any worker.
    * @arg deadline the priority of the task. (added to the previous task's deadline).
    */
    Task& fork(std::function<void()> action, size_t worker=0, uint64_t deadline=0);

    /**
    * Schedules a task to occur concurrently with the previous task with the given priority
    *  on the associated worker thread. Further tasks will not wait for this task to complete.
    * @arg action the task to perform.
    * @arg worker the associated worker thread. Worker=0 means any worker.
    * @arg deadline the absolute priority of the task. (not added to the previous task's deadline).
    */
    Task& forkAbsolute(std::function<void()> action, size_t worker=0, uint64_t deadline=0);

    /**
    * Returns the associated task_t* of this object and then !empties this object!
    *  This function is intended to be used by the RFUS implementing classes ONLY!
    * @return the root of the built task_t* structure.
    */
    task_t* getTask();

private:
    task_t *root, *or_root, *leaf;
};

}
