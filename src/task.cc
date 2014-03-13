// This file is part of RFUS (Rich's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

#include "task.hpp"
#include "detail/join_semaphore.hpp"

using namespace rfus;

task_t::task_t(std::function<void()> action, size_t worker, uint64_t deadline)
    : action(action)
    , priority(deadline)
    , continuation(nullptr)
    , join(nullptr)
    , worker(worker)
    , next(nullptr)
{
}

task_t::~task_t()
{
    if(join != nullptr)
    {
        // Given that join is not nullptr we have to handle
        // a few cases:
        //    1. We are not the last task_t in a chain of also tasks
        //         which all have the same continuation/join
        //       * Delete our next
        //
        //    2. We are the last task_t in a chain of also tasks
        //         which all have the same continuation/join
        //      * Delete our next
        //      * Delete our Continuation

        if(next != nullptr)
        {
            bool delete_continue = join != next->join;
            delete next;
            if(delete_continue && continuation != nullptr)
                delete continuation;
        }
        else if(continuation != nullptr)
        {
            delete continuation;
        }
        
    }
    else
    {
        // Given that join is nullptr we simply need
        // to do the following:
        //     1. Delete our next, if we have one
        //     2. Delete our continuation, if we have one
        if(next != nullptr) delete next;
        if(continuation != nullptr) delete continuation;
    }
}
