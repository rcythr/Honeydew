// This file is part of Honeydew
// Honeydew is licensed under the MIT LICENSE. See the LICENSE file for more info.
#pragma once

namespace honeydew
{
namespace detail
{

template<>
struct Pipeline<void>
{
    Task task;

    /**
    * Constructor that extends the current pipeline.
    *   to create a new pipeline use Pipeline::start() instead.
    */
    Pipeline(Task&& task)
        : task(std::forward<Task>(task))
    {
    }

    /**
    * Adds a stage the current pipeline.
    *  The return value of the previous stage will be passed into this task. After this the pipeline
    *  must be closed because the task returns no value.
    * @arg action the task to perform
    * @arg worker the thread to run this action upon.
    * @arg deadline the priority of this task. This value is relative to the previous task..
    */ 
    Pipeline<void> then(std::function<void()> action, size_t worker=0, uint64_t deadline=0)
    {
        task.then(action, worker, deadline);
        return Pipeline<void>(std::move(task));
    }
    
    /**
    * Adds a stage the current pipeline.
    *  The return value of the previous stage will be passed into this task. After this the pipeline
    *  must be closed because the task returns no value.
    * @arg action the task to perform
    * @arg worker the thread to run this action upon.
    * @arg deadline the priority of this task. This value is absolute.
    */ 
    Pipeline<void> then_absolute(std::function<void()> action, size_t worker=0, uint64_t deadline=0)
    {
        task.then_absolute(action, worker, deadline);
        return Pipeline<void>(std::move(task));
    }
    
    /**
    * Completes this task at the same time as the previous task. Further tasks will wait for all
    *   tasks in the also to be completed before being run.
    * @arg action the task to perform.
    * @arg worker the thread to run this task upon.
    * @arg deadline the priority of this task. Relative to the previous task.
    */    
    Pipeline<void> also(std::function<void()> action, size_t worker=0, uint64_t deadline=0)
    {
        task.also(action, worker, deadline);
        return Pipeline<void>(std::move(task));
    }

    /**
    * Completes this task at the same time as the previous task. Further tasks will wait for all
    *   tasks in the also to be completed before being run.
    * @arg action the task to perform.
    * @arg worker the thread to run this task upon.
    * @arg deadline the priority of this task. This value is absolute. 
    */    
    Pipeline<void> also_absolute(std::function<void()> action, size_t worker=0, uint64_t deadline=0)
    {
        task.also_absolute(action, worker, deadline);
        return Pipeline<void>(std::move(task));
    }
    
    /**
    * Completes this task at the same time as the previous task. Further tasks will not wait for all
    *   tasks in the also to be completed before being run.
    * @arg action the task to perform.
    * @arg worker the thread to run this task upon.
    * @arg deadline the priority of this task. Relative to the previous task.
    */    
    template<typename ReturnValue>
    Pipeline<void> fork(std::function<ReturnValue()> action, size_t worker=0, uint64_t deadline=0)
    {
        task.fork([=] () { action(); }, worker, deadline);
        return Pipeline<void>(std::move(task));
    }

    /**
    * Completes this task at the same time as the previous task. Further tasks will not wait for all
    *   tasks in the also to be completed before being run.
    * @arg action the task to perform.
    * @arg worker the thread to run this task upon.
    * @arg deadline the priority of this task. This value is absolute. 
    */    
    template<typename ReturnValue>
    Pipeline<void> fork_absolute(std::function<ReturnValue()> action, size_t worker=0, uint64_t deadline=0)
    {
        task.fork_absolute([=] () { action(); }, worker, deadline);
        return Pipeline<void>(std::move(task));
    }

    /**
    * Adds a stage the current pipeline.
    *  The return value of the previous stage will be passed into this task and the result will be passed
    *  to the next stage in the pipeline.
    * @arg action the task to perform
    * @arg worker the thread to run this action upon.
    * @arg deadline the priority of this task. This value is relative to the previous task.
    */ 
    template<typename ReturnType>
    Pipeline<ReturnType> then(std::function<ReturnType()> action, size_t worker=0, uint64_t deadline=0)
    {
        ReturnType* result = new ReturnType();
        task.then([=] () { *result = action(); }, worker, deadline);
        return Pipeline<ReturnType>(std::move(task), result);
    }

    /**
    * Adds a stage the current pipeline.
    *  The return value of the previous stage will be passed into this task and the result will be passed
    *  to the next stage in the pipeline.
    * @arg action the task to perform
    * @arg worker the thread to run this action upon.
    * @arg deadline the priority of this task. This value is absolute.
    */ 
    template<typename ReturnType>
    Pipeline<ReturnType> then_abolute(std::function<ReturnType()> action, size_t worker=0, uint64_t deadline=0)
    {
        ReturnType* result = new ReturnType();
        task.then_absolute(worker, [=] () { *result = action(); }, deadline);
        return Pipeline<ReturnType>(std::move(task), result);
    }

    /**
    * Completes this task at the same time as the previous task. Further tasks will wait for all
    *   tasks in the also to be completed before being run.
    * @arg action the task to perform.
    * @arg worker the thread to run this task upon.
    * @arg deadline the priority of this task. Relative to the previous task.
    */    
    template<typename ReturnType>
    Pipeline<ReturnType> also(std::function<ReturnType()> action, size_t worker=0, uint64_t deadline=0)
    {
        ReturnType* result = new ReturnType();
        task.also([=] () { *result = action(); }, worker, deadline);
        return Pipeline<ReturnType>(std::move(task), result);
    }

    /**
    * Completes this task at the same time as the previous task. Further tasks will wait for all
    *   tasks in the also to be completed before being run.
    * @arg action the task to perform.
    * @arg worker the thread to run this task upon.
    * @arg deadline the priority of this task. This value is absolute. 
    */    
    template<typename ReturnType>
    Pipeline<ReturnType> also_absolute(std::function<ReturnType()> action, size_t worker=0, uint64_t deadline=0)
    {
        ReturnType* result = new ReturnType();
        task.also_absolute([=] () { *result = action(); }, worker, deadline);
        return Pipeline<ReturnType>(std::move(task), result);
    }

    /**
    * Closes the pipeline early without creating resources needed to remember the return type of this call.
    * @arg action the action to execute
    * @arg worker the worker to run this action upon.
    * @arg deadline a priority value to use. This value is relative to the previous task.
    * @return a task_t* object for a Honeydew. 
    */
    template<typename ReturnType>
    task_t* close_with(std::function<ReturnType()> action, size_t worker=0, uint64_t deadline=0)
    {
        task.then([=] () { action(); }, worker, deadline);
        return task.close();
    }
    
    /**
    * Closes the pipeline early without creating resources needed to remember the return type of this call.
    * @arg action the action to execute
    * @arg worker the worker to run this action upon.
    * @arg deadline a priority value to use. This value is absolute.
    * @return a task_t* object for a Honeydew. 
    */
    template<typename ReturnType>
    task_t* close_with_absolute(std::function<ReturnType()> action, size_t worker=0, uint64_t deadline=0)
    {
        task.then_absolute([=] () { action(); }, worker, deadline);
        return task.close();
    }
    
    /**
    * Closes the current pipeline
    * @return a task_t* which can be used by a Honeydew.
    */
    task_t* close()
    {
        return task.close();
    }
};

}
}
