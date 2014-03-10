// This file is part of RFUS (Rcythr's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

#pragma once

#include "task.h"

template<typename PrevReturn>
struct Pipeline
{
    Task& task;
    PrevReturn* prev_result;

    Pipeline(Task& task, PrevReturn* result)
        : task(task)
        , prev_result(result)
    {
    }

    template<typename ReturnType>
    static Pipeline<ReturnType> start(std::function<ReturnType()> action)
    {
        ReturnType* result = new ReturnType();
        Task task([=] () { *result = action(); });
        return Pipeline<ReturnType>(task, result);
    }

    template<typename ReturnType>
    Pipeline<ReturnType> then(std::function<ReturnType(PrevReturn)> action)
    {
        ReturnType* result = new ReturnType();
        task.then([=] () { *result = action(prev_result); delete prev_result; });
        return Pipeline<ReturnType>(task, result);
    }

    template<>
    Pipeline<void> then(std::function<void(PrevReturn)> action)
    {
        task.then([=] () { action(prev_result); delete prev_result; });
        return Pipeline<void>(task);
    }

    Task& close()
    {
        task.then([=] () { delete prev_result; });
        return task;
    }

};

template<>
struct Pipeline<void>
{
    Task& task;

    Pipeline(Task& task)
        : task(task)
    {
    }

    Task& close()
    {
        return task;
    }
}
