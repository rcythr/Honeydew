// This file is part of RFUS (Rich's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

#pragma once

#include "rfus.hpp"
#include "helpers/task_wrapper.hpp"

namespace rfus
{

/**
* A helper class which wraps a Task such that the new Task:
*       1. Runs the given Task
*       2. [Optionally] If that task runs successfully, posts an on_success task.
*       3. [Optionally] If that task throws an ExceptionType exception, posts an on_failure task, passing the exception.
*
* To catch *all* exceptions use OutcomeTask (helpers/outcome_task.hpp) instead.
*/
template<typename ExceptionType>
class ExceptionTask
{
public:
    
    /**
    * Creates a new Exception task.
    * @arg rfus the RFUS instance to post on_success and on_failure tasks to, if needed.
    * @arg functor the function to wrap.
    * @arg worker the thread to run the wrapped function upon.
    * @arg priority the priority of the wrapped task.
    */
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

    /**
    * Specifies the Task to post if this task does not throw an exception.
    * @arg other the task to move into this wrapper.
    * @return reference to this for daisy chains
    */
    ExceptionTask& on_success(Task&& other)
    {
        on_success_task = std::forward<Task>(other);
        return *this;
    }
    
    /**
    * Specifies the Task to post if this task throws an ExceptionType exception.
    *   because this is a prebuilt task, exception cannot be passed in.
    * @arg other the task to move into this wrapper.
    * @return reference to this for daisy chains
    */
    ExceptionTask& on_failure(Task&& other)
    {
        on_failure_task = std::forward<Task>(other);
        return *this;
    }

    /**
    * Specifies the Task to post if this task throws an ExceptionType exception.
    *   exception will be passed into the given handler. 
    * @arg handler the function used to handle the exception.
    * @arg handler_worker the worker for the handler task.
    * @arg handler_priority the priority of the handler task.
    * @return reference to this for daisy chains
    */
    ExceptionTask& on_failure(std::function<void(ExceptionType&)> handler, size_t handler_worker=0, uint64_t handler_priority=0)
    {
        this->handler = handler;
        this->handler_worker = handler_worker;
        this->handler_priority = handler_priority;
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
        
        // Required to prevent capture of this instead of the actual fields.
        //  Dammit C++.
        std::function<void()>& functor_copy = functor;
        RFUSInterface*& rfus_copy = rfus;                
        std::function<void(ExceptionType&)>& handler_copy = handler;
        size_t& worker_copy = worker;
        uint64_t& priority_copy = priority;

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
    
private:
    RFUSInterface* rfus;
    std::function<void()> functor;
    size_t worker;
    uint64_t priority;
    Task on_success_task;
    Task on_failure_task;
    std::function<void(ExceptionType&)> handler;
    size_t handler_worker;
    uint64_t handler_priority;
};

}
