// This file is part of RFUS (Rich's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

#pragma once

#include <rfus/task.hpp>

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
    * Deleted copy constructor.
    */
    Task(const Task& other) = delete;

    /**
    * Move constructor.
    */
    Task(Task&& other);

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
    Task& then_absolute(std::function<void()> action, size_t worker=0,  uint64_t deadline=0);

    /*
    * Adds another task_t heirarchy as a then relationship to the end of this Task structure.
    *  The root of the resulting structure is then returned.
    * @arg other the root of another task heirarchy.
    * @return the root of this Task structure.
    */
    task_t* then_close(task_t* other);
    template<typename TaskType> task_t* then_close(TaskType&& other) { return then_close(other.close()); }

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
    Task& also_absolute(std::function<void()> action, size_t worker=0, uint64_t deadline=0);

    /**
    * Adds another task_t* structure as an also relationship to this task. The other heirarchy
    *  will take place at the same time as the last level of tasks in this hierarchy. This task is then
    *  closed
    * @arg other the root of the other task heirarchy.
    */
    task_t* also_close(task_t* other);
    template<typename TaskType> task_t* also_close(TaskType&& other) { return also_close(other.close()); }
    
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
    Task& fork_absolute(std::function<void()> action, size_t worker=0, uint64_t deadline=0);
   
    /**
    * Adds another task heirarchy as a forked task onto this heirarchy.
    * @arg other the other task heirarchy to fork from the current leaf of this heirarchy.
    * @return this task.
    */ 
    Task& fork(task_t* other);
    template<typename TaskType> Task& fork(TaskType& other) { return fork(other.close()); }

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
