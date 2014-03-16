// This file is part of Honeydew
// Honeydew is licensed under the MIT LICENSE. See the LICENSE file for more info.
#pragma once

namespace honeydew
{
namespace detail
{

template<class PrevReturn>
struct Pipeline
{
    Task task;
    PrevReturn* prev_result;

    /**
    * Constructor that extends the current pipeline.
    *   to create a new pipeline use Pipeline::start() instead.
    */
    Pipeline(Task&& task, PrevReturn* result)
        : task(std::forward<Task>(task))
        , prev_result(result)
    {
    }

    /**
    * Adds a stage the current pipeline.
    *  The return value of the previous stage will be passed into this task. After this the pipeline
    *  must be closed because the task returns no value.
    * @arg action the task to perform
    * @arg worker the thread to run this action upon.
    * @arg deadline the priority of this task. This value is relative to the previous task..
    */ 
    Pipeline<void> then(std::function<void(PrevReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& prev_result_ref = prev_result;
        task.then([=] () { action(*prev_result_ref); delete prev_result_ref; }, worker, deadline);
        return Pipeline<void>(std::move(task));
    }
    
    /**
    * Adds a stage the current pipeline.
    *  The return value of the previous stage will be passed into this task. After this the pipeline
    *  must be closed because the task returns no value.
    * @arg action the task to perform
    * @arg worker the thread to run this action upon.
    * @arg deadline the priority of this task. This value is absolute.
    */ 
    Pipeline<void> then_absolute(std::function<void(PrevReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& prev_result_ref = prev_result;
        task.then_absolute([=] () { action(*prev_result_ref); delete prev_result_ref; }, worker, deadline);
        return Pipeline<void>(std::move(task));
    }

    /**
    * Adds a stage the current pipeline.
    *  The return value of the previous stage will be passed into this task and the result will be passed
    *  to the next stage in the pipeline.
    * @arg action the task to perform
    * @arg worker the thread to run this action upon.
    * @arg deadline the priority of this task. This value is relative to the previous task.
    */ 
    template<typename ReturnType>
    Pipeline<ReturnType> then(std::function<ReturnType(PrevReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& prev_result_ref = prev_result;
        ReturnType* result = new ReturnType();
        task.then([=] () { *result = action(*prev_result_ref); delete prev_result_ref; }, worker, deadline);
        return Pipeline<ReturnType>(std::move(task), result);
    }
   
    /**
    * Adds a stage the current pipeline.
    *  The return value of the previous stage will be passed into this task and the result will be passed
    *  to the next stage in the pipeline.
    * @arg action the task to perform
    * @arg worker the thread to run this action upon.
    * @arg deadline the priority of this task. This value is absolute.
    */ 
    template<typename ReturnType>
    Pipeline<ReturnType> then_absolute(std::function<ReturnType(PrevReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& prev_result_ref = prev_result;
        ReturnType* result = new ReturnType();
        task.then_absolute(worker, [=] () { *result = action(*prev_result_ref); delete prev_result_ref; }, deadline);
        return Pipeline<ReturnType>(std::move(task), result);
    }

    /**
    * Closes the pipeline early without creating resources needed to remember the return type of this call.
    * @arg action the action to execute
    * @arg worker the worker to run this action upon.
    * @arg deadline a priority value to use. This value is relative to the previous task.
    * @return a task_t* object for a Honeydew. 
    */
    template<typename ReturnType>
    task_t* close_with(std::function<ReturnType(PrevReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& prev_result_ref = prev_result;
        task.then([=] () { action(*prev_result_ref); delete prev_result_ref; }, worker, deadline);
        return task.close();
    }
    
    /**
    * Closes the pipeline early without creating resources needed to remember the return type of this call.
    * @arg action the action to execute
    * @arg worker the worker to run this action upon.
    * @arg deadline a priority value to use. This value is absolute.
    * @return a task_t* object for a Honeydew. 
    */
    template<typename ReturnType>
    task_t* close_with_absolute(std::function<ReturnType(PrevReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& prev_result_ref = prev_result;
        task.then_absolute([=] () { action(*prev_result_ref); delete prev_result_ref; }, worker, deadline);
        return task.close();
    }
    
    /**
    * Starts a new fork. The given function will run alongside the following functions until after a corresponding join.
    * @arg action the action to execute
    * @arg worker the worker to run this action upon.
    * @arg deadline a priority value to use. This value is relative to the previous priority.
    * @return A forked pipeline object. This is used to daisy chain calls.
    */
    template<typename ReturnType>
    ForkedPipeline<ReturnType> split(std::function<ReturnType(PrevReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& prev_result_ref = prev_result;
        join_semaphore_t* join_sem = new join_semaphore_t(1);
        task.then([=] () { 
            action(*prev_result_ref); 
            if(join_sem->decrement() == 0)
            {
                delete join_sem;
                delete prev_result_ref;
            }
        }, worker, deadline);
        return ForkedPipeline<ReturnType>(std::move(task), prev_result, join_sem);
    }

    /**
    * Starts a new fork. The given function will run alongside the following functions until after a corresponding join.
    * @arg action the action to execute
    * @arg worker the worker to run this action upon
    * @arg deadline a priority value to use. This value is absolute.
    * @return A forked pipeline object. This is used to daisy chain calls.
    */
    template<typename ReturnType>
    ForkedPipeline<ReturnType> split_absolute(std::function<ReturnType(PrevReturn)> action, size_t worker=0, uint64_t deadline=0)
    {
        auto& prev_result_ref = prev_result;
        join_semaphore_t* join_sem = new join_semaphore_t(1);
        task.then_absolute([=] () { 
            action(*prev_result_ref);
            if(join_sem->decrement() == 0)
            {
                delete join_sem;
                delete prev_result_ref;
            }
        }, worker, deadline);
        return ForkedPipeline<ReturnType>(std::move(task), prev_result, join_sem);
    }

    /**
    * Ends the pipeline early. The final return value will be discarded.
    * @return the task_t* object which can be inserted into a Honeydew.
    */    
    task_t* close()
    {
        task.then([=] () { delete prev_result; });
        return task.close();
    }

};

}
}
