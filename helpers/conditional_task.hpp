#pragma once

#include "rfus.hpp"
#include "task.hpp"

struct ConditionalTask
{
    RFUSInterface* rfus;

    std::function<bool()> conditional;
    size_t worker;
    uint64_t priority;

    Task on_true;
    Task on_false;

    ConditionalTask(RFUSInterface* rfus, std::function<bool()> condition, size_t worker=0, uint64_t priority=0)
        : rfus(rfus)
        , conditional(condition)
        , on_true(nullptr)
        , on_false(nullptr)
    {
    }

    ConditionalTask& on_true(Task&& other)
    {
        on_true = std::forward<Task>(other);
        return *this;
    }

    Task& on_false(Task&& other)
    {
        on_false = std::forward<Task>(other);
        return *this;
    }

    task_t* close()
    {
        task_t* true_task = on_true.getTask();
        task_t* false_task = on_false.getTask();

        return Task([=] () {
            if(conditional())
                if(true_task) { rfus->post(true_task); //TODO: Delete false_task }
            else
                if(on_false) { rfus->post(on_false); //TODO: Delete true_task }
        }, worker, priority).getTask();
    }
};
