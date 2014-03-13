// This file is part of RFUS (Rich's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

#pragma once

#include <rfus/rfus.hpp>
#include <rfus/helpers/task_wrapper.hpp>

namespace rfus
{

/**
* Wraps a given bool returning function into a task such that:
*   1. The given bool returning function is called.
*   2. [Optional] If the returning function returns true, calls the on_true task.
*   3. [Optional] If the returning function returns false, calls the on_false task.
*/
class ConditionalTask
{
public:

    /**
    * Creates a new conditional task using the given condition function and worker/priority.
    * @arg rfus the RFUS to post on_true and on_false to as needed.
    * @arg condition the conditional function to call when this task resolves
    * @arg worker the worker thread to run the conditional task on.
    * @arg priority the priority of the conditional task.
    */
    ConditionalTask(RFUSInterface* rfus, std::function<bool()> condition, size_t worker=0, uint64_t priority=0)
        : rfus(rfus)
        , conditional(condition)
    {
    }

    /**
    * Sets the task that is evaluated when the condition resolves to true.
    * @arg other the Task wrapper to steal internals from.
    * @return a reference to this for daisy chaining.
    */
    ConditionalTask& on_true(Task&& other)
    {
        on_true_task = std::forward<Task>(other);
        return *this;
    }

    /**
    * Sets the task that is evaluated when the condition resolves to false.
    * @arg other the Task wrapper to steal internals from.
    * @return a reference to this for daisy chaining.
    */
    ConditionalTask& on_false(Task&& other)
    {
        on_false_task = std::forward<Task>(other);
        return *this;
    }

    /**
    * Closes this task and returns the underlying task_t*.
    * @return the task_t* generated and ready to be pushed to a RFUS.
    */
    task_t* close()
    {
        task_t* true_task = on_true_task.close();
        task_t* false_task = on_false_task.close();

        // Necessary to prevent capture of 'this'
        //   Dammit C++.
        RFUSInterface*& rfus_copy = rfus;
        std::function<bool()>& conditional_copy = conditional;

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

private:
    RFUSInterface* rfus;
    std::function<bool()> conditional;
    size_t worker;
    uint64_t priority;
    Task on_true_task;
    Task on_false_task;
};

}
