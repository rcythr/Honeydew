#pragma once

#include "rfus.hpp"
#include "helpers/task_wrapper.hpp"

namespace rfus
{

/**
* A helper class which wraps a Task such that the new Task:
*       1. Runs the given Task
*       2. [Optionally] If that task runs successfully, posts an on_success task.
*       3. [Optionally] If that task throws *any* exception, posts an on_failure task.
*
* To catch a specific exception and recieve it in the on_failure task use ExceptionTask (helpers/exception_task.hpp) instead.
*/
class OutcomeTask
{
public:

    /**
    * Creates a new OutcomeTask.
    * @arg rfus The RFUS instance to post to in the event of a success or failure.
    * @arg functor the functor to wrap.
    * @arg worker the thread to run the functor on.
    * @arg priority the priority of the wrapped task.
    */
    OutcomeTask(RFUSInterface* rfus, std::function<void()> functor, size_t worker=0, uint64_t priority=0)
        : rfus(rfus)
        , functor(functor)
        , worker(worker)
        , priority(priority)
    {
    }

    /**
    * Specifies the Task to post if this task does not throw an exception.
    * @arg other the task to move into this wrapper.
    * @return reference to this for daisy chains
    */
    OutcomeTask& on_success(Task&& other)
    {
        on_success_task = std::forward<Task>(other);
        return *this;
    }

    /**
    * Specifies the Task to post if this task throws _any_ exception.
    * @arg other the task to move into this wrapper.
    * @return reference to this for daisy chains
    */
    OutcomeTask& on_failure(Task&& other)
    {
        on_failure_task = std::forward<Task>(other);
        return *this;
    }

    /**
    * Wraps up the task, on_success, and on_failure tasks into a task that can be posted
    *  to a RFUS.
    * @return The RFUS postable task of this wrapper.
    */
    task_t* close()
    {
        task_t* success_task = on_success_task.close();
        task_t* failure_task = on_failure_task.close();

        // This is necessary to prevent capture of 'this' into the lambda.
        //   Dammit C++.
        RFUSInterface*& rfus_copy = rfus;
        std::function<void()>& functor_copy = functor;

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

private:
    RFUSInterface* rfus;
    std::function<void()> functor;
    size_t worker;
    uint64_t priority;
    Task on_success_task;
    Task on_failure_task;
};

}
