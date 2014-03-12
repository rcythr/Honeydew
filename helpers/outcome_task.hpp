#pragma once

#include "rfus.hpp"
#include "helpers/task_wrapper.hpp"

namespace rfus
{

struct OutcomeTask
{
    RFUSInterface* rfus;

    std::function<void()> functor;
    size_t worker;
    uint64_t priority;
    
    Task on_success_task;
    Task on_failure_task;

    OutcomeTask(RFUSInterface* rfus, std::function<void()> functor, size_t worker=0, uint64_t priority=0)
        : rfus(rfus)
        , functor(functor)
        , worker(worker)
        , priority(priority)
    {
    }

    OutcomeTask& on_success(Task&& other)
    {
        on_success_task = std::forward<Task>(other);
        return *this;
    }

    OutcomeTask& on_failure(Task&& other)
    {
        on_failure_task = std::forward<Task>(other);
        return *this;
    }

    task_t* close()
    {
        task_t* success_task = on_success_task.close();
        task_t* failure_task = on_failure_task.close();

        // This is necessary to prevent capture of 'this' into the lambda.
        //   Dammit C++.
        RFUSInterface* rfus_copy = rfus;
        std::function<void()> functor_copy = functor;

        return Task([=] () {
            try
            {
                functor_copy();
                if(success_task) rfus_copy->post(success_task); 
                if(failure_task) delete failure_task;
            }
            catch(...)
            {
                if(failure_task) rfus_copy->post(failure_task); 
                if(success_task) delete success_task;
            }
        }, worker, priority).close();
    }
};

}
