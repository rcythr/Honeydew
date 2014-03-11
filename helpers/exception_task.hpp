#pragma once

#include "rfus.hpp"
#include "task.hpp"

template<typename ExceptionType>
struct ExceptionTask
{
    std::function<void()> functor;
    size_t worker;
    uint64_t priority;

    Task on_success;

    Task on_exception;
    std::function<void(ExceptionType&)> handler;
    size_t handler_worker;
    uint64_t handler_priority;

    ExceptionTask(std:function<void()> functor, size_t worker=0, uint64_t priority=0)
        : functor(functor)
        , worker(worker)
        , priority(priority)
        , handler(nullptr)
        , handler_worker(0)
        , handler_priority(0)
    {
    }

    ExceptionTask& on_success(Task&& other)
    {
        on_success = std::forward<Task>(other);
        return *this;
    }

    ExceptionTask& on_failure(Task&& other)
    {
        on_failure = std::forward<Task>(other);
        return *this;
    }

    ExceptionTask& on_failure(std::function<void(ExceptioNType&)> handler, size_t handler_worker=0, uint64_t handler_priority=0)
    {
        this->handler = handler;
        this->handler_worker = handler_worker;
        this->handler_priority = handler_priority;
        return *this;
    }

    task_t* close()
    {
        task_t* success_task = on_success.getTask();
        if(handler)
        {
            return Task([] () {
                try
                {
                    functor();
                    rfus->post(success_task);
                }
                catch(ExceptionType e)
                {
                    //TODO: Delete success_task
                    rfus->post(Task(std::bind(handler, e), handler_worker, handler_priority).getTask());
                }
            }, worker, priority).getTask();
        }
        else
        {
            task_t* failure_task = on_failure.getTask();
            return Task([] () {
                try
                {
                    functor();
                    if(success_task) { rfus->post(success_task); //TODO: Delete failure_task }
                }
                catch(ExceptionType e)
                {
                    if(failure_task) { rfus->post(failure_task); //TODO: Delete success_task }
                }
            }, worker, priority).getTask();
        }
    }
};
