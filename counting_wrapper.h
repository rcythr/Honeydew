// This file is part of RFUS (Rcythr's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

#pragma once

#include <atomic>

/**
* A simple wrapper for queues that adds .size() functionality.
*  It should be noted that the size functionality is not strictly thread safe
*  because a sub-queue's size can change before the size is updated. However,
*  for most applications strict thread safety on size is not necessary because
*  it will only affect where tasks get scheduled.
*/
template<typename QueueType>
class CountingWrapper
{
private:

    /**
    * Constructs a CountingWrapper with size of zero.
    */
    CountingWrapper()
        : n(0)
    {
    }

    /**
    * Adds this task to the internal queue and increments the size.
    * @param task the task to add.
    * @pre None
    * @post The task is added and the size is increased by 1.
    */
    void push(typename QueueType::value_type* task)
    {
        q.push(task);
        ++n;
    }

    /**
    * Removes up to step elements from the queue and decrements the size accordingly.
    * @param step the number of elements to try and remove.
    * @param result a pointer to a location to store the first output task.
    * @pre None
    * @post Up to step tasks is removed from the internal queue and size is decremented accordingly.
    * @return the number of tasks effectively removed.
    */
    size_t pop(size_t step, typename QueueType::value_type **result)
    {
        step = q.pop(step, result);
        n.fetch_sub(step);
        return step;
    }

    /**
    * Returns the current size of the underlying queue.
    *   (This function is not strictly atomic)
    */
    size_t size() const
    {
        return n.load();
    }

private:
    QueueType q;
    std::atomic<size_t> n;
};
