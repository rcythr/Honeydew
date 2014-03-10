// This file is part of RFUS (Rcythr's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

#pragma once

#include "task.h"
#include "join_semaphore.h"

#include <functional>
#include <utility>
#include <type_traits>

namespace rcythr
{

namespace detail
{

template<class PrevReturn> struct Pipeline;
template<class PrevReturn> struct ForkedPipeline;

template<class ForkReturn>
struct ForkedPipeline
{
    static_assert(std::is_same<void, ForkReturn>::value, "Cannot fork a pipeline after a void");

    Task task;
    ForkReturn* prev_return;
    join_semaphore_t* join_sem;

    ForkedPipeline(Task&& task, ForkReturn* result, join_semaphore_t* join_sem)
        : task(std::forward<Task>(task))
        , prev_return(result)
        , join_sem(join_sem)
    {
    }

    template<typename ReturnValue>
    ForkedPipeline<ForkReturn> also(std::function<ReturnValue(ForkReturn)> action, uint64_t deadline=0)
    {
        join_sem->increment();
        task.also([=] () { 
            action(*prev_return);
            if(join_sem->decrement())
            {
                delete join_sem;
                delete prev_return;
            }
        }, deadline);
        return ForkedPipeline<ForkReturn>(std::move(task), prev_return, join_sem);
    }

    template<typename ReturnValue>
    ForkedPipeline<ForkReturn> also(size_t worker, std::function<ReturnValue(ForkReturn)> action, uint64_t deadline=0)
    {
        join_sem->increment();
        task.also(worker, [=] () { 
            action(*prev_return);
            if(join_sem->decrement())
            {
                delete join_sem;
                delete prev_return;
            }
        }, deadline);
        return ForkedPipeline<ForkReturn>(std::move(task), prev_return, join_sem);
    }

    template<typename ReturnValue>
    ForkedPipeline<ForkReturn> alsoAbsolute(std::function<ReturnValue(ForkReturn)> action, uint64_t deadline)
    {
        join_sem->increment();
        task.alsoAbsolute([=] () { 
            action(*prev_return);
            if(join_sem->decrement())
            {
                delete join_sem;
                delete prev_return;
            }
        }, deadline);
        return ForkedPipeline<ForkReturn>(std::move(task), prev_return, join_sem);
    }

    template<typename ReturnValue>
    ForkedPipeline<ForkReturn> alsoAbsolute(size_t worker, std::function<ReturnValue(ForkReturn)> action, uint64_t deadline)
    {
        join_sem->increment();
        task.alsoAbsolute(worker, [=] () { 
            action(*prev_return);
            if(join_sem->decrement())
            {
                delete join_sem;
                delete prev_return;
            }
        }, deadline);
        return ForkedPipeline<ForkReturn>(std::move(task), prev_return, join_sem);
    }

    template<typename ReturnType>
    Pipeline<ReturnType> join(std::function<ReturnType(ForkReturn)> action, uint64_t deadline=0)
    {
        ReturnType* result = new ReturnType();

        join_sem->increment();
        task.also([=] () { 
            *result = action(*prev_return);
            if(join_sem->decrement())
            {
                delete join_sem;
                delete prev_return;
            }
        }, deadline);
        return Pipeline<ReturnType>(std::move(task), result);
    }
    
    template<typename ReturnType>
    Pipeline<ReturnType> join(size_t worker, std::function<ReturnType(ForkReturn)> action, uint64_t deadline=0)
    {
        ReturnType* result = new ReturnType();

        join_sem->increment();
        task.also(worker, [=] () { 
            *result = action(*prev_return);
            if(join_sem->decrement())
            {
                delete join_sem;
                delete prev_return;
            }
        }, deadline);
        return Pipeline<ReturnType>(std::move(task), result);
    }
    
    template<typename ReturnType>
    Pipeline<ReturnType> joinAbsolute(std::function<ReturnType(ForkReturn)> action, uint64_t deadline)
    {
        ReturnType* result = new ReturnType();

        join_sem->increment();
        task.alsoAbsolute([=] () { 
            *result = action(*prev_return);
            if(join_sem->decrement())
            {
                delete join_sem;
                delete prev_return;
            }
        }, deadline);
        return Pipeline<ReturnType>(std::move(task), result);
    }
    
    template<typename ReturnType>
    Pipeline<ReturnType> joinAbsolute(size_t worker, std::function<ReturnType(ForkReturn)> action, uint64_t deadline)
    {
        ReturnType* result = new ReturnType();

        join_sem->increment();
        task.alsoAbsolute(worker, [=] () { 
            *result = action(*prev_return);
            if(join_sem->decrement())
            {
                delete join_sem;
                delete prev_return;
            }
        }, deadline);
        return Pipeline<ReturnType>(std::move(task), result);
    }

    task_t* close()
    {
        return task.getTask();
    }
};

template<>
struct Pipeline<void>
{
    Task task;

    Pipeline(Task&& task)
        : task(std::forward<Task>(task))
    {
    }

    task_t* close()
    {
        return task.getTask();
    }
};

template<class PrevReturn>
struct Pipeline
{
    Task task;
    PrevReturn* prev_result;

    Pipeline(Task&& task, PrevReturn* result)
        : task(std::forward<Task>(task))
        , prev_result(result)
    {
    }

    Pipeline<void> then(std::function<void(PrevReturn)> action, uint64_t deadline=0)
    {
        task.then([=] () { action(*prev_result); delete prev_result; }, deadline);
        return Pipeline<void>(std::move(task));
    }

    Pipeline<void> then(size_t worker, std::function<void(PrevReturn)> action, uint64_t deadline=0)
    {
        task.then(worker, [=] () { action(*prev_result); delete prev_result; }, deadline);
        return Pipeline<void>(std::move(task));
    }
    
    Pipeline<void> thenAbsolute(std::function<void(PrevReturn)> action, uint64_t deadline)
    {
        task.thenAbsolute([=] () { action(*prev_result); delete prev_result; }, deadline);
        return Pipeline<void>(std::move(task));
    }

    Pipeline<void> thenAbsolute(size_t worker, std::function<void(PrevReturn)> action, uint64_t deadline)
    {
        task.thenAbsolute(worker, [=] () { action(*prev_result); delete prev_result; }, deadline);
        return Pipeline<void>(std::move(task));
    }

    template<typename ReturnType>
    Pipeline<ReturnType> then(std::function<ReturnType(PrevReturn)> action, uint64_t deadline=0)
    {
        ReturnType* result = new ReturnType();
        task.then([=] () { *result = action(*prev_result); delete prev_result; }, deadline);
        return Pipeline<ReturnType>(std::move(task), result);
    }

    template<typename ReturnType>
    Pipeline<ReturnType> then(size_t worker, std::function<ReturnType(PrevReturn)> action, uint64_t deadline=0)
    {
        ReturnType* result = new ReturnType();
        task.then(worker, [=] () { *result = action(*prev_result); delete prev_result; }, deadline);
        return Pipeline<ReturnType>(std::move(task), result);
    }
    
    template<typename ReturnType>
    Pipeline<ReturnType> thenAbsolute(std::function<ReturnType(PrevReturn)> action, uint64_t deadline)
    {
        ReturnType* result = new ReturnType();
        task.thenAbsolute([=] () { *result = action(*prev_result); delete prev_result; }, deadline);
        return Pipeline<ReturnType>(std::move(task), result);
    }

    template<typename ReturnType>
    Pipeline<ReturnType> thenAbolute(size_t worker, std::function<ReturnType(PrevReturn)> action, uint64_t deadline)
    {
        ReturnType* result = new ReturnType();
        task.then(worker, [=] () { *result = action(*prev_result); delete prev_result; }, deadline);
        return Pipeline<ReturnType>(std::move(task), result);
    }

    template<typename ReturnType>
    task_t* closeWith(std::function<ReturnType(PrevReturn)> action, uint64_t deadline=0)
    {
        task.then([=] () { action(*prev_result); delete prev_result; }, deadline);
        return task.getTask();
    }

    template<typename ReturnType>
    task_t* closeWith(size_t worker, std::function<ReturnType(PrevReturn)> action, uint64_t deadline=0)
    {
        task.then(worker, [=] () { action(*prev_result); delete prev_result; }, deadline);
        return task.getTask();
    }
    
    template<typename ReturnType>
    task_t* closeWithAbsolute(std::function<ReturnType(PrevReturn)> action, uint64_t deadline)
    {
        task.thenAbsolute([=] () { action(*prev_result); delete prev_result; }, deadline);
        return task.getTask();
    }

    template<typename ReturnType>
    task_t* closeWithAbsolute(size_t worker, std::function<ReturnType(PrevReturn)> action, uint64_t deadline)
    {
        task.thenAbsolute(worker, [=] () { action(*prev_result); delete prev_result; }, deadline);
        return task.getTask();
    }

    template<typename ReturnType>
    ForkedPipeline<ReturnType> fork(std::function<ReturnType(PrevReturn)> action, uint64_t deadline=0)
    {
        join_semaphore_t* join_sem = new join_semaphore_t(1);
        ReturnType* result = new ReturnType();
        task.then([=] () { *result = action(*prev_result); delete prev_result; }, deadline);
        return ForkedPipeline<ReturnType>(std::move(task), result, join_sem);
    }
    
    template<typename ReturnType>
    ForkedPipeline<ReturnType> fork(size_t worker, std::function<ReturnType(PrevReturn)> action, uint64_t deadline=0)
    {
        join_semaphore_t* join_sem = new join_semaphore_t(1);
        ReturnType* result = new ReturnType();
        task.then(worker, [=] () { *result = action(*prev_result); delete prev_result; }, deadline);
        return ForkedPipeline<ReturnType>(std::move(task), result, join_sem);
    }

    template<typename ReturnType>
    ForkedPipeline<ReturnType> forkAbsolute(std::function<ReturnType(PrevReturn)> action, uint64_t deadline)
    {
        join_semaphore_t* join_sem = new join_semaphore_t(1);
        ReturnType* result = new ReturnType();
        task.thenAbsolute([=] () { *result = action(*prev_result); delete prev_result; }, deadline);
        return ForkedPipeline<ReturnType>(std::move(task), result, join_sem);
    }
    
    template<typename ReturnType>
    ForkedPipeline<ReturnType> forkAbsolute(size_t worker, std::function<ReturnType(PrevReturn)> action, uint64_t deadline)
    {
        join_semaphore_t* join_sem = new join_semaphore_t(1);
        ReturnType* result = new ReturnType();
        task.thenAbsolute(worker, [=] () { *result = action(*prev_result); delete prev_result; }, deadline);
        return ForkedPipeline<ReturnType>(std::move(task), result, join_sem);
    }
    
    task_t* close()
    {
        task.then([=] () { delete prev_result; });
        return task.getTask();
    }

};

}

struct Pipeline
{
    template<typename ReturnType>
    static detail::Pipeline<ReturnType> start(std::function<ReturnType()> action, uint64_t deadline=0)
    {
        ReturnType* result = new ReturnType();
        Task task([=] () { *result = action(); }, deadline);
        return detail::Pipeline<ReturnType>(std::move(task), result);
    }
    
    template<typename ReturnType>
    static detail::Pipeline<ReturnType> start(size_t worker, std::function<ReturnType()> action, uint64_t deadline=0)
    {
        ReturnType* result = new ReturnType();
        Task task(worker, [=] () { *result = action(); }, deadline);
        return detail::Pipeline<ReturnType>(std::move(task), result);
    }
};

}
