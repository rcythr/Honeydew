#pragma once

#include "rfus.hpp"
#include "helpers/task_wrapper.hpp"

namespace rfus
{

struct ConditionalTask
{
    RFUSInterface* rfus;

    std::function<bool()> conditional;
    size_t worker;
    uint64_t priority;

    Task on_true_task;
    Task on_false_task;

    ConditionalTask(RFUSInterface* rfus, std::function<bool()> condition, size_t worker=0, uint64_t priority=0)
        : rfus(rfus)
        , conditional(condition)
    {
    }

    ConditionalTask& on_true(Task&& other)
    {
        on_true_task = std::forward<Task>(other);
        return *this;
    }

    ConditionalTask& on_false(Task&& other)
    {
        on_false_task = std::forward<Task>(other);
        return *this;
    }

    task_t* close()
    {
        task_t* true_task = on_true_task.close();
        task_t* false_task = on_false_task.close();

        // Necessary to prevent capture of 'this'
        //   Dammit C++.
        RFUSInterface* rfus_copy = rfus;
        std::function<bool()> conditional_copy = conditional;

        return Task([=] () {
            if(conditional_copy())
            {
                if(true_task) rfus_copy->post(true_task); 
                if(false_task) delete false_task; 
            }
            else
            {
                if(false_task) rfus_copy->post(false_task); 
                if(true_task) delete true_task;
            }
        }, worker, priority).close();
    }
};

}
