// This file is part of RFUS (Rcythr's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

#pragma once

#include "task.h"
#include <functional>
#include <utility>

namespace rcythr
{

namespace detail
{

template<class PrevReturn> struct Pipeline;

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

    Pipeline<void> then(std::function<void(PrevReturn)> action)
    {
        task.then([=] () { action(*prev_result); delete prev_result; });
        return Pipeline<void>(std::move(task));
    }

    Pipeline<void> then(std::function<void(PrevReturn)> action, uint64_t deadline)
    {
        task.then([=] () { action(*prev_result); delete prev_result; }, deadline);
        return Pipeline<void>(std::move(task));
    }

    Pipeline<void> then(size_t worker, std::function<void(PrevReturn)> action)
    {
        task.then(worker, [=] () { action(*prev_result); delete prev_result; });
        return Pipeline<void>(std::move(task));
    }

    Pipeline<void> then(size_t worker, std::function<void(PrevReturn)> action, uint64_t deadline)
    {
        task.then(worker, [=] () { action(*prev_result); delete prev_result; }, deadline);
        return Pipeline<void>(std::move(task));
    }

    template<typename ReturnType>
    Pipeline<ReturnType> then(std::function<ReturnType(PrevReturn)> action)
    {
        ReturnType* result = new ReturnType();
        task.then([=] () { *result = action(*prev_result); delete prev_result; });
        return Pipeline<ReturnType>(std::move(task), result);
    }

    template<typename ReturnType>
    Pipeline<ReturnType> then(std::function<ReturnType(PrevReturn)> action, uint64_t deadline)
    {
        ReturnType* result = new ReturnType();
        task.then([=] () { *result = action(*prev_result); delete prev_result; }, deadline);
        return Pipeline<ReturnType>(std::move(task), result);
    }

    template<typename ReturnType>
    Pipeline<ReturnType> then(size_t worker, std::function<ReturnType(PrevReturn)> action)
    {
        ReturnType* result = new ReturnType();
        task.then(worker, [=] () { *result = action(*prev_result); delete prev_result; });
        return Pipeline<ReturnType>(std::move(task), result);
    }

    template<typename ReturnType>
    Pipeline<ReturnType> then(size_t worker, std::function<ReturnType(PrevReturn)> action, uint64_t deadline)
    {
        ReturnType* result = new ReturnType();
        task.then(worker, [=] () { *result = action(*prev_result); delete prev_result; }, deadline);
        return Pipeline<ReturnType>(std::move(task), result);
    }

    template<typename ReturnType>
    task_t* closeWith(std::function<ReturnType(PrevReturn)> action)
    {
        task.then([=] () { action(*prev_result); delete prev_result; });
        return task.getTask();
    }

    template<typename ReturnType>
    task_t* closeWith(std::function<ReturnType(PrevReturn)> action, uint64_t deadline)
    {
        task.then([=] () { action(*prev_result); delete prev_result; }, deadline);
        return task.getTask();
    }

    template<typename ReturnType>
    task_t* closeWith(size_t worker, std::function<ReturnType(PrevReturn)> action)
    {
        task.then(worker, [=] () { action(*prev_result); delete prev_result; });
        return task.getTask();
    }

    template<typename ReturnType>
    task_t* closeWith(size_t worker, std::function<ReturnType(PrevReturn)> action, uint64_t deadline)
    {
        task.then(worker, [=] () { action(*prev_result); delete prev_result; }, deadline);
        return task.getTask();
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
    static detail::Pipeline<ReturnType> start(std::function<ReturnType()> action)
    {
        ReturnType* result = new ReturnType();
        Task task([=] () { *result = action(); });
        return detail::Pipeline<ReturnType>(std::move(task), result);
    }
};

}
