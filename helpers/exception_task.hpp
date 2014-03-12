#pragma once

#include "rfus.hpp"
#include "helpers/task_wrapper.hpp"

namespace rfus
{

template<typename ExceptionType>
struct ExceptionTask
{
    RFUSInterface* rfus;
    std::function<void()> functor;
    size_t worker;
    uint64_t priority;

    Task on_success_task;
    Task on_failure_task;

    std::function<void(ExceptionType&)> handler;
    size_t handler_worker;
    uint64_t handler_priority;

    ExceptionTask(RFUSInterface* rfus, std::function<void()> functor, size_t worker=0, uint64_t priority=0)
        : rfus(rfus)
        , functor(functor)
        , worker(worker)
        , priority(priority)
        , handler(nullptr)
        , handler_worker(0)
        , handler_priority(0)
    {
    }

    ExceptionTask& on_success(Task&& other)
    {
        on_success_task = std::forward<Task>(other);
        return *this;
    }

    ExceptionTask& on_failure(Task&& other)
    {
        on_failure_task = std::forward<Task>(other);
        return *this;
    }

    ExceptionTask& on_failure(std::function<void(ExceptionType&)> handler, size_t handler_worker=0, uint64_t handler_priority=0)
    {
        this->handler = handler;
        this->handler_worker = handler_worker;
        this->handler_priority = handler_priority;
        return *this;
    }

    task_t* close()
    {
        task_t* success_task = on_success_task.close();
        
        // Required to prevent capture of this instead of the actual fields.
        //  Dammit C++.
        std::function<void()> functor_copy = functor;
        RFUSInterface* rfus_copy = rfus;                
        std::function<void(ExceptionType&)> handler_copy = handler;
        size_t worker_copy = worker;
        uint64_t priority_copy = priority;

        if(handler)
        {
            return Task([=] () {
                try
                {
                    functor_copy();
                    rfus_copy->post(success_task);
                }
                catch(ExceptionType e)
                {
                    rfus_copy->post(Task(std::bind(handler_copy, e), worker_copy, priority_copy));
                    delete success_task;
                }
            }, worker, priority).close();
        }
        else
        {
            task_t* failure_task = on_failure_task.close();
            return Task([=] () {
                try
                {
                    functor_copy();
                    if(success_task) rfus_copy->post(success_task); 
                    if(failure_task) delete failure_task;
                }
                catch(ExceptionType e)
                {
                    if(failure_task) rfus_copy->post(failure_task); 
                    if(success_task) delete success_task;
                }
            }, worker, priority).close();
        }
    }
};

}
