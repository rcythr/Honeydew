#pragma once

#include <functional>
#include <cstdint>
#include <vector>

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

    /**
    * Constructs a new task object with a priority-less action.
    * @arg action function to schedule and call on any thread.
    */
    Task(std::function<void()> action);

    /**
    * Constructs a new task object with a given deadline.
    * @arg function to schedule and call on any thread
    * @arg deadline the absolute deadline (priority) of this call.
    */
    Task(std::function<void()> action, uint64_t deadline);

    /**
    * Constructs a new task object with a priority-less action.
    * @arg worker the associated thread on which to run this action.
    * @arg action function to schedule and call on the associated worker thread.
    */
    Task(size_t worker, std::function<void()> action);

    /**
    * Constructs a new task object with a given deadline.
    * @arg worker the associated thread on which to run this action.
    * @arg function to schedule and call on the associated worker thread.
    * @arg deadline the absolute deadline (priority) of this call.
    */
    Task(size_t worker, std::function<void()> action, uint64_t deadline);

    /**
    * Cleans up the internals of this object if necessary.
    */
    ~Task();

    Task& then(std::function<void()> action);
    Task& then(std::function<void()> action, uint64_t deadline);
    Task& thenAbsolute(std::function<void()> action, uint64_t deadline);

    Task& then(size_t worker, std::function<void()> action);
    Task& then(size_t worker, std::function<void()> action, uint64_t deadline);
    Task& thenAbsolute(size_t worker, std::function<void()> action, uint64_t deadline);

    Task& also(std::function<void()> action);
    Task& also(std::function<void()> action, uint64_t deadline);
    Task& alsoAbsolute(std::function<void()> action, uint64_t deadline);

    Task& also(size_t worker, std::function<void()> action);
    Task& also(size_t worker, std::function<void()> action, uint64_t deadline);
    Task& alsoAbsolute(size_t worker, std::function<void()> action, uint64_t deadline);

    /**
    * Returns the associated task_t* of this object and then !empties this object!
    * @return the root of the built task_t* structure.
    */
    task_t* getTask();

private:
    task_t *root, *or_root, *leaf;
};
