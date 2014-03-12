#pragma once

#include "task.hpp"

namespace rfus
{

/**
* Class that allows for easy building of task_t* structures.
*  Usage is expected to be via daisy-chaining of function calls
*  onto this object like Task(func1).then(func2).also(func3);
*/
class Task
{
public:

    /**
    * Constructs a new, empty Task wrapper object.
    */
    Task();
        
    /**
    * Constructs a new task object with a given deadline.
    * @arg function to schedule and call on the associated worker thread.
    * @arg worker the associated thread on which to run this action. Worker=0 means any worker.
    * @arg deadline the absolute deadline (priority) of this call.
    */
    Task(std::function<void()> action, size_t worker=0, uint64_t deadline=0);

    /**
    * Move constructor.
    */
    Task(Task&& other);

    /**
    * Deleted copy constructor.
    */
    Task(const Task& other) = delete;


    /**
    * Initializes a previously uninitialized task. This function throws std::runtime_error
    *  if the task was previously initialized.
    */
    void init(std::function<void()> action=0, size_t worker=0, uint64_t deadline=0);

    /**
    * Deleted copy assignment.
    */
    Task& operator=(const Task& other) = delete;

    // Move assignment.
    Task& operator=(Task&& other);

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
    task_t* close();

private:
    task_t *root, *or_root, *leaf;
};

}
