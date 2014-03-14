// This file is part of Honeydew
// Honeydew is licensed under the MIT LICENSE. See the LICENSE file for more info.

#pragma once

#include <atomic>

namespace honeydew
{

/**
* Simple wrapper for an atomic integer that serves as the semaphore
*  in concurrently running tasks that then must be joined.
*/
class join_semaphore_t
{
public:

    /**
    * Creates a new instance of this join_semaphore with the given initial value.
    * @param the initial value of the atomic count.
    */
    join_semaphore_t(unsigned int initial_value)
    {
        n.store(initial_value);
    }

    /**
    * Increments the number of tasks in this semaphore.
    */
    void increment()
    {
        n.fetch_add(1);
    }

    /**
    * Decrements the number of tasks remaining in this semaphore.
    * @return the number of tasks remaining.
    */
    size_t decrement()
    {
        return n.fetch_sub(1) - 1;
    }

private:
    std::atomic<unsigned int> n;
};

}
