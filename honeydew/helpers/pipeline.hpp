// This file is part of Honeydew
// Honeydew is licensed under the MIT LICENSE. See the LICENSE file for more info.
#pragma once

#include <honeydew/helpers/task_wrapper.hpp>
#include <honeydew/detail/join_semaphore.hpp>

#include <utility>
#include <type_traits>

namespace honeydew
{

namespace detail
{

template<class PrevReturn> struct Pipeline;
template<class PrevReturn> struct ForkedPipeline;
template<typename TupleType, typename PrevReturn, size_t Current> struct TupledPipeline;

}
}

#include <honeydew/helpers/pipelines/void.hpp>
#include <honeydew/helpers/pipelines/forked.hpp>
#include <honeydew/helpers/pipelines/nonvoid.hpp>

namespace honeydew
{

/**
* Struct containing static methods to create a pipeline of tasks.
*/
struct Pipeline
{
    /**
    * Constructs new pipeline with the given function to start with
    * @arg worker the worker to run the action upon
    * @arg action the action to place into the pipeline.
    * @arg deadline an optional parameter for a priority value.
    * @return A pipeline. Used to daisy chain calls.
    */
    template<typename ReturnType>
    static detail::Pipeline<ReturnType> start(std::function<ReturnType()> action, size_t worker=0, uint64_t deadline=0)
    {
        ReturnType* result = new ReturnType();
        Task task([=] () { *result = action(); }, worker, deadline);
        return detail::Pipeline<ReturnType>(std::move(task), result);
    }
    
    /**
    * Constructs new forked pipeline with the given function to start with
    * @arg worker the worker to run the action upon
    * @arg action the action to place into the pipeline.
    * @arg deadline an optional parameter for a priority value.
    * @return A forked pipeline. Used to daisy chain calls.
    */
    template<typename ReturnType>
    static detail::ForkedPipeline<ReturnType> start_forked(std::function<ReturnType()> action, size_t worker=0, uint64_t deadline=0)
    {
        join_semaphore_t* join_sem = new join_semaphore_t(1);
        ReturnType* result = new ReturnType();
        Task task([=] () { *result = action(); }, worker, deadline);
        return detail::ForkedPipeline<ReturnType>(std::move(task), result, join_sem);
    }
};

}
