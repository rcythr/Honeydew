// This file is part of RFUS (Rich's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

#pragma once

#include "helpers/task_wrapper.hpp"
#include "join_semaphore.hpp"

#include <utility>
#include <type_traits>

namespace rfus
{

namespace detail
{

template<class PrevReturn> struct Pipeline;
template<class PrevReturn> struct ForkedPipeline;

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
    Pipeline<void> thenAbsolute(std::function<void()> action, size_t worker=0, uint64_t deadline=0)
    {
        task.thenAbsolute(action, worker, deadline);
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
    Pipeline<void> alsoAbsolute(std::function<void()> action, size_t worker=0, uint64_t deadline=0)
    {
        task.alsoAbsolute(action, worker, deadline);
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
    Pipeline<void> forkAbsolute(std::function<ReturnValue()> action, size_t worker=0, uint64_t deadline=0)
    {
        task.forkAbsolute([=] () { action(); }, worker, deadline);
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
    Pipeline<ReturnType> thenAbolute(std::function<ReturnType()> action, size_t worker=0, uint64_t deadline=0)
    {
        ReturnType* result = new ReturnType();
        task.then(worker, [=] () { *result = action(); }, deadline);
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
    Pipeline<ReturnType> alsoAbsolute(std::function<ReturnType()> action, size_t worker=0, uint64_t deadline=0)
    {
        ReturnType* result = new ReturnType();
        task.alsoAbsolute([=] () { *result = action(); }, worker, deadline);
        return Pipeline<ReturnType>(std::move(task), result);
    }

    /**
    * Closes the pipeline early without creating resources needed to remember the return type of this call.
    * @arg action the action to execute
    * @arg worker the worker to run this action upon.
    * @arg deadline a priority value to use. This value is relative to the previous task.
    * @return a task_t* object for a RFUS. 
    */
    template<typename ReturnType>
    task_t* closeWith(std::function<ReturnType()> action, size_t worker=0, uint64_t deadline=0)
    {
        task.then([=] () { action(); }, worker, deadline);
        return task.close();
    }
    
    /**
    * Closes the pipeline early without creating resources needed to remember the return type of this call.
    * @arg action the action to execute
    * @arg worker the worker to run this action upon.
    * @arg deadline a priority value to use. This value is absolute.
    * @return a task_t* object for a RFUS. 
    */
    template<typename ReturnType>
    task_t* closeWithAbsolute(std::function<ReturnType()> action, size_t worker=0, uint64_t deadline=0)
    {
        task.thenAbsolute([=] () { action(); }, worker, deadline);
        return task.close();
    }
    
    /**
    * Closes the current pipeline
    * @return a task_t* which can be used by a RFUS.
    */
    task_t* close()
    {
        return task.close();
    }
};

template<class ForkReturn>
struct ForkedPipeline
{

    Task task;
    ForkReturn* prev_return;
    join_semaphore_t* join_sem;

    /**
    * Extends the current forked pipeline. To create a new Forked pipeline use
    *  Pipeline::startForked() instead.
    */
    ForkedPipeline(Task&& task, ForkReturn* result, join_semaphore_t* join_sem)
        : task(std::forward<Task>(task))
        , prev_return(result)
        , join_sem(join_sem)
    {
    }

    /*
    * Adds another concurrently running task to be performed with the value returned at the start of the fork.
    *  the return value from the action here will be discarded.
    * @arg action the function to run. Return value is discarded.
    * @arg worker the worker to run this task upon.
    * @arg deadline the priority of this task. This value is relative to the previous task..
    */
    template<typename ReturnValue>
    ForkedPipeline<ForkReturn> also(std::function<ReturnValue(ForkReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& join_sem_ref = join_sem;
        auto& prev_return_ref = prev_return;

        join_sem->increment();
        task.also([=] () { 
            action(*prev_return_ref);
            if(join_sem_ref->decrement() == 0)
            {
                delete join_sem_ref;
                delete prev_return_ref;
            }
        }, worker, deadline);
        return ForkedPipeline<ForkReturn>(std::move(task), prev_return, join_sem);
    }
    
    /*
    * Adds another concurrently running task to be performed with the value returned at the start of the fork.
    *  the return value from the action here will be discarded.
    * @arg action the function to run. Return value is discarded.
    * @arg worker the worker to run this task upon.
    * @arg deadline the priority of this task. This value is absolute.
    */
    template<typename ReturnValue>
    ForkedPipeline<ForkReturn> alsoAbsolute(std::function<ReturnValue(ForkReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& join_sem_ref = join_sem;
        auto& prev_return_ref = prev_return;

        join_sem->increment();
        task.alsoAbsolute([=] () { 
            action(*prev_return_ref);
            if(join_sem_ref->decrement() == 0)
            {
                delete join_sem_ref;
                delete prev_return_ref;
            }
        }, worker, deadline);
        return ForkedPipeline<ForkReturn>(std::move(task), prev_return, join_sem);
    }

    /**
    * Adds a task to be completed that will not cause the pipeline to wait.
    *  This task can start at the same time as the previous task, but will not force the next task to wait for it.
    * @arg action the task to be completed.
    * @arg worker the worker thread to run this upon.
    * @arg deadline the priority of this task. This value is relative to the previous priority.
    */
    template<typename ReturnValue>
    ForkedPipeline<ForkReturn> fork(std::function<ReturnValue(ForkReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& join_sem_ref = join_sem;
        auto& prev_return_ref = prev_return;

        join_sem->increment();
        task.fork([=] () {
            action(*prev_return_ref);
            if(join_sem_ref->decrement() == 0)
            {
                delete join_sem_ref;
                delete prev_return_ref;
            }
        }, worker, deadline);
        return ForkedPipeline<ForkReturn>(std::move(task), prev_return, join_sem);
    }

    /**
    * Adds a task to be completed that will not cause the pipeline to wait.
    *  This task can start at the same time as the previous task, but will not force the next task to wait for it.
    * @arg action the task to be completed.
    * @arg worker the worker thread to run this upon.
    * @arg deadline the priority of this task. This value is absolute.
    */
    template<typename ReturnValue>
    ForkedPipeline<ForkReturn> forkAbsolute(std::function<ReturnValue(ForkReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& join_sem_ref = join_sem;
        auto& prev_return_ref = prev_return;

        join_sem->increment();
        task.forkAbsolute([=] () {
            action(*prev_return_ref);
            if(join_sem_ref->decrement() == 0)
            {
                delete join_sem_ref;
                delete prev_return_ref;
            }
        }, worker, deadline);
        return ForkedPipeline<ForkReturn>(std::move(task), prev_return, join_sem);
    }

    /**
    * Joins this fork back into a normal pipeline. The return value of this function will be passed into
    *  the next stage of the pipeline as normal.
    * @arg action the task to perform.
    * @arg worker the thread to run this task upon.
    * @arg deadline the priority of this task. This value is relative to the previous task.
    */
    Pipeline<void> join(std::function<void(ForkReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& join_sem_ref = join_sem;
        auto& prev_return_ref = prev_return;

        join_sem->increment();
        task.also([=] () { 
            action(*prev_return_ref);
            if(join_sem_ref->decrement() == 0)
            {
                delete join_sem_ref;
                delete prev_return_ref;
            }
        }, worker, deadline);
        return Pipeline<void>(std::move(task));
    }
    
    /**
    * Joins this fork back into a normal pipeline. The return value of this function will be passed into
    *  the next stage of the pipeline as normal.
    * @arg action the task to perform.
    * @arg worker the thread to run this task upon.
    * @arg deadline the priority of this task. This value is absolute.
    */
    Pipeline<void> joinAbsolute(std::function<void(ForkReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& join_sem_ref = join_sem;
        auto& prev_return_ref = prev_return;

        join_sem->increment();
        task.alsoAbsolute([=] () { 
            action(*prev_return_ref);
            if(join_sem_ref->decrement() == 0)
            {
                delete join_sem_ref;
                delete prev_return_ref;
            }
        }, worker, deadline);
        return Pipeline<void>(std::move(task));
    }
    
    /**
    * Joins this fork back into a normal pipeline. The return value of this function will be passed into
    *  the next stage of the pipeline as normal.
    * @arg action the task to perform.
    * @arg worker the thread to run this task upon.
    * @arg deadline the priority of this task. This value is relative to the previous task.
    */
    template<typename ReturnType>
    Pipeline<ReturnType> join(std::function<ReturnType(ForkReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        ReturnType* result = new ReturnType();
        auto& join_sem_ref = join_sem;
        auto& prev_return_ref = prev_return;

        join_sem->increment();
        task.also([=] () { 
            *result = action(*prev_return_ref);
            if(join_sem_ref->decrement() == 0)
            {
                delete join_sem_ref;
                delete prev_return_ref;
            }
        }, worker, deadline);
        return Pipeline<ReturnType>(std::move(task), result);
    }
    
    /**
    * Joins this fork back into a normal pipeline. The return value of this function will be passed into
    *  the next stage of the pipeline as normal.
    * @arg action the task to perform.
    * @arg worker the thread to run this task upon.
    * @arg deadline the priority of this task. This value is absolute.
    */
    template<typename ReturnType>
    Pipeline<ReturnType> joinAbsolute(std::function<ReturnType(ForkReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        ReturnType* result = new ReturnType();
        auto& join_sem_ref = join_sem;
        auto& prev_return_ref = prev_return;

        join_sem->increment();
        task.alsoAbsolute([=] () { 
            *result = action(*prev_return_ref);
            if(join_sem_ref->decrement() == 0)
            {
                delete join_sem_ref;
                delete prev_return_ref;
            }
        }, worker, deadline);
        return Pipeline<ReturnType>(std::move(task), result);
    }

    /*
    * Closes the pipeline.
    * @return a task_t* which can be used by a RFUS.
    */
    task_t* close()
    {
        return task.close();
    }
};


template<class PrevReturn>
struct Pipeline
{
    Task task;
    PrevReturn* prev_result;

    /**
    * Constructor that extends the current pipeline.
    *   to create a new pipeline use Pipeline::start() instead.
    */
    Pipeline(Task&& task, PrevReturn* result)
        : task(std::forward<Task>(task))
        , prev_result(result)
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
    Pipeline<void> then(std::function<void(PrevReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& prev_result_ref = prev_result;
        task.then([=] () { action(*prev_result_ref); delete prev_result_ref; }, worker, deadline);
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
    Pipeline<void> thenAbsolute(std::function<void(PrevReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& prev_result_ref = prev_result;
        task.thenAbsolute([=] () { action(*prev_result_ref); delete prev_result_ref; }, worker, deadline);
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
    Pipeline<ReturnType> then(std::function<ReturnType(PrevReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& prev_result_ref = prev_result;
        ReturnType* result = new ReturnType();
        task.then([=] () { *result = action(*prev_result_ref); delete prev_result_ref; }, worker, deadline);
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
    Pipeline<ReturnType> thenAbolute(std::function<ReturnType(PrevReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& prev_result_ref = prev_result;
        ReturnType* result = new ReturnType();
        task.then(worker, [=] () { *result = action(*prev_result_ref); delete prev_result_ref; }, deadline);
        return Pipeline<ReturnType>(std::move(task), result);
    }

    /**
    * Closes the pipeline early without creating resources needed to remember the return type of this call.
    * @arg action the action to execute
    * @arg worker the worker to run this action upon.
    * @arg deadline a priority value to use. This value is relative to the previous task.
    * @return a task_t* object for a RFUS. 
    */
    template<typename ReturnType>
    task_t* closeWith(std::function<ReturnType(PrevReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& prev_result_ref = prev_result;
        task.then([=] () { action(*prev_result_ref); delete prev_result_ref; }, worker, deadline);
        return task.close();
    }
    
    /**
    * Closes the pipeline early without creating resources needed to remember the return type of this call.
    * @arg action the action to execute
    * @arg worker the worker to run this action upon.
    * @arg deadline a priority value to use. This value is absolute.
    * @return a task_t* object for a RFUS. 
    */
    template<typename ReturnType>
    task_t* closeWithAbsolute(std::function<ReturnType(PrevReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& prev_result_ref = prev_result;
        task.thenAbsolute([=] () { action(*prev_result_ref); delete prev_result_ref; }, worker, deadline);
        return task.close();
    }
    
    /**
    * Starts a new fork. The given function will run alongside the following functions until after a corresponding join.
    * @arg action the action to execute
    * @arg worker the worker to run this action upon.
    * @arg deadline a priority value to use. This value is relative to the previous priority.
    * @return A forked pipeline object. This is used to daisy chain calls.
    */
    template<typename ReturnType>
    ForkedPipeline<ReturnType> split(std::function<ReturnType(PrevReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& prev_result_ref = prev_result;
        join_semaphore_t* join_sem = new join_semaphore_t(1);
        task.then([=] () { 
            action(*prev_result_ref); 
            if(join_sem->decrement() == 0)
            {
                delete join_sem;
                delete prev_result_ref;
            }
        }, worker, deadline);
        return ForkedPipeline<ReturnType>(std::move(task), prev_result, join_sem);
    }

    /**
    * Starts a new fork. The given function will run alongside the following functions until after a corresponding join.
    * @arg action the action to execute
    * @arg worker the worker to run this action upon
    * @arg deadline a priority value to use. This value is absolute.
    * @return A forked pipeline object. This is used to daisy chain calls.
    */
    template<typename ReturnType>
    ForkedPipeline<ReturnType> splitAbsolute(std::function<ReturnType(PrevReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& prev_result_ref = prev_result;
        join_semaphore_t* join_sem = new join_semaphore_t(1);
        task.thenAbsolute([=] () { 
            action(*prev_result_ref);
            if(join_sem->decrement() == 0)
            {
                delete join_sem;
                delete prev_result_ref;
            }
        }, worker, deadline);
        return ForkedPipeline<ReturnType>(std::move(task), prev_result, join_sem);
    }

    /**
    * Ends the pipeline early. The final return value will be discarded.
    * @return the task_t* object which can be inserted into a RFUS.
    */    
    task_t* close()
    {
        task.then([=] () { delete prev_result; });
        return task.close();
    }

};

}

/**
* Struct containing static methods to create a pipeline of tasks.
*/
struct Pipeline
{
    /**
    * Constructs new pipeline with the given function to start with
    * @arg worker the worker to run the action upon
    * @arg action the action to place into the pipeline.
    * @arg deadline an optional parameter for a priority value.
    * @return A pipeline. Used to daisy chain calls.
    */
    template<typename ReturnType>
    static detail::Pipeline<ReturnType> start(std::function<ReturnType()> action, size_t worker=0, uint64_t deadline=0)
    {
        ReturnType* result = new ReturnType();
        Task task([=] () { *result = action(); }, worker, deadline);
        return detail::Pipeline<ReturnType>(std::move(task), result);
    }
    
    /**
    * Constructs new forked pipeline with the given function to start with
    * @arg worker the worker to run the action upon
    * @arg action the action to place into the pipeline.
    * @arg deadline an optional parameter for a priority value.
    * @return A forked pipeline. Used to daisy chain calls.
    */
    template<typename ReturnType>
    static detail::ForkedPipeline<ReturnType> startFork(std::function<ReturnType()> action, size_t worker=0, uint64_t deadline=0)
    {
        join_semaphore_t* join_sem = new join_semaphore_t(1);
        ReturnType* result = new ReturnType();
        Task task([=] () { *result = action(); }, worker, deadline);
        return detail::ForkedPipeline<ReturnType>(std::move(task), result, join_sem);
    }
};

}
