// This file is part of RFUS (Rich's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

#include "rfus.hpp"

#include "detail/queue.hpp"
#include "detail/priority_queue.hpp"
#include "detail/counting_wrapper.hpp"
#include "detail/join_semaphore.hpp"

#include <thread>
#include <vector>

using namespace rfus;

// Global State Management
RFUSInterface* rfus::RFUS;

typedef CountingWrapper<Queue<task_t>> CountingQueue;
typedef CountingWrapper<PriorityQueue<task_t>> PriorityCountingQueue;

template<typename QueueType>
struct RFUSImpl : public RFUSInterface
{
    typedef std::function<size_t(std::atomic_int_fast32_t&,task_t*,QueueType*,size_t)> FindQueueFunc;

    RFUSImpl(size_t num_threads, size_t step_size, FindQueueFunc findQueue)
        : findQueue(findQueue)
        , num_threads(num_threads)
        , runningCount(0)
    {
        queues = new QueueType[num_threads];
        for(size_t i=0; i < num_threads; ++i)
        {
            threads.emplace_back(std::bind(&RFUSImpl::run, this, &queues[i], step_size));
        }
    }

    void run(QueueType* q, size_t step_size)
    {
        task_t *next;
        while(1)
        {
            task_t* task = nullptr;
            q->pop(step_size, &task);
            while(task != nullptr)
            {
                try
                {
                    task->action();
                }
                catch(...)
                {
                    // Do nothing
                }

                if(task->join != nullptr)
                {
                    size_t prev_n = task->join->decrement();
                    if(prev_n == 1)
                    {
                        delete task->join;
                        task->join = nullptr;

                        if(task->continuation != nullptr)
                        {
                            post(task->continuation);
                        }
                    }
                }
                else
                {
                    if(task->continuation != nullptr)
                    {
                        post(task->continuation);
                    }
                }

                next = task->next;
                task->next = nullptr;
                task->continuation = nullptr;
                delete task;
                task = next;
            }

            std::this_thread::yield();
        }
    }

    virtual RFUSInterface* post(task_t* task)
    {
        task_t* next;
        while(task != nullptr)
        {
            next = task->next;
            task->next = nullptr;
            if(task->worker == 0)
            {
                queues[findQueue(runningCount, task, queues, num_threads)].push(task);
            }
            else
            {
                queues[task->worker % num_threads].push(task);
            }
            task = next;
        }
        return this;
    }

    std::vector<std::thread> threads;
    FindQueueFunc findQueue;
    QueueType* queues;
    size_t num_threads;
    std::atomic_int_fast32_t runningCount;
};

/**
* Creates a new RFUS of the given type.
* @param type the type of the RFUS to create. This cooresponds to how resource-less events are scheduled.
* @param num_threads the number of workers to create. This affects the number of independent work queues.
*                       if the number of resources > num_threads some resources will share a thread.
* @param step_size the maximum number of events each worker removes from the queue at a time. 0 is infinite.
*/
RFUSInterface* rfus::createRFUS(RFUSType type, size_t num_threads, size_t step_size)
{
    switch(type)
    {
    case ROUND_ROBIN:
        return new RFUSImpl<Queue<task_t>>(num_threads, step_size,
        [] (std::atomic_int_fast32_t& running_count, task_t* task, Queue<task_t>* queues, size_t num_queues) {
            return running_count.fetch_add(1) % num_queues;
        });
    case ROUND_ROBIN_WITH_PRIORITY:
        return new RFUSImpl<PriorityQueue<task_t>>(num_threads, step_size,
        [] (std::atomic_int_fast32_t& running_count, task_t* task, PriorityQueue<task_t>* queues, size_t num_queues) {
            return running_count.fetch_add(1) % num_queues;
        });
    case LEAST_BUSY:
        return new RFUSImpl<CountingQueue>(num_threads, step_size,
        [] (std::atomic_int_fast32_t& running_count, task_t* task, CountingQueue* queues, size_t num_queues) {
            size_t least_busy = 0;
            size_t least_busy_amt = queues[0].size();
            for(size_t i=1; i < num_queues; ++i)
            {
                size_t amt = queues[i].size();
                if(amt < least_busy_amt)
                {
                    least_busy_amt = amt;
                    least_busy = i;
                }
            }
            return least_busy;
        });
    case LEAST_BUSY_WITH_PRIORITY:
        return new RFUSImpl<PriorityCountingQueue>(num_threads, step_size,
        [] (std::atomic_int_fast32_t& running_count, task_t* task, PriorityCountingQueue* queues, size_t num_queues) {
            size_t least_busy = 0;
            size_t least_busy_amt = queues[0].size();
            for(size_t i=1; i < num_queues; ++i)
            {
                size_t amt = queues[i].size();
                if(amt < least_busy_amt)
                {
                    least_busy_amt = amt;
                    least_busy = i;
                }
            }
            return least_busy;
        });
    }
    return nullptr;
}
