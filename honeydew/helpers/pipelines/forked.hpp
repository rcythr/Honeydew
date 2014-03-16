// This file is part of Honeydew
// Honeydew is licensed under the MIT LICENSE. See the LICENSE file for more info.
#pragma once

namespace honeydew
{
namespace detail
{

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
    ForkedPipeline<ForkReturn> also_absolute(std::function<ReturnValue(ForkReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& join_sem_ref = join_sem;
        auto& prev_return_ref = prev_return;

        join_sem->increment();
        task.also_absolute([=] () { 
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
    ForkedPipeline<ForkReturn> fork_absolute(std::function<ReturnValue(ForkReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& join_sem_ref = join_sem;
        auto& prev_return_ref = prev_return;

        join_sem->increment();
        task.fork_absolute([=] () {
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
    Pipeline<void> join_absolute(std::function<void(ForkReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& join_sem_ref = join_sem;
        auto& prev_return_ref = prev_return;

        join_sem->increment();
        task.also_absolute([=] () { 
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
    Pipeline<ReturnType> join_absolute(std::function<ReturnType(ForkReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        ReturnType* result = new ReturnType();
        auto& join_sem_ref = join_sem;
        auto& prev_return_ref = prev_return;

        join_sem->increment();
        task.also_absolute([=] () { 
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
    * @return a task_t* which can be used by a Honeydew.
    */
    task_t* close()
    {
        return task.close();
    }
};

}
}
