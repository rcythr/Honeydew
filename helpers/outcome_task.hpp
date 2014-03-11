#pragma once

#include "rfus.hpp"
#include "task.hpp"

struct OutcomeTask
{
    RFUSInterface* rfus;

    std::function<void()> functor;
    size_t worker;
    uint64_t priority;
    
    Task on_success;
    Task on_failure;

    OutcomeTask(RFUSInterface* rfus, std::function<void()> functor, size_t worker=0, uint64_t priority=0)
        : rfus(rfus)
        , functor(functor)
        , worker(worker)
        , priority(priority)
    {
    }

    OutcomeTask& on_success(Task&& other)
    {
        on_success = std::forward<Task>(other);
        return *this;
    }

    OutcomeTask& on_failure(Task&& other)
    {
        on_failure = std::forward<Task>(other);
        return *this;
    }

    task_t* close()
    {
        task_t* success_task = on_success.getTask();
        task_t* failure_task = on_failure.getTask();
        return Task([=] () {
            try
            {
                functor();
                if(success_task) { rfus->post(success_task); //TODO; Delete failure_task }
            }
            catch(...)
            {
                if(failure_task) { rfus->post(failure_task); //TODO: Delete success_task }
            }
        }, worker, priority).getTask();
    }
};
