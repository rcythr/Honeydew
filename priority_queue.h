// This file is part of RFUS (Rich's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

#pragma once

#include <mutex>
#include <condition_variable>

namespace rfus
{

/**
* A locking priority queue that will store elements in increasing order
*  according to their ->priority. Lower priority value is highest priority.
*  This is done to allow deadline times to be the priority.
*/
template<typename T>
class PriorityQueue
{
public:

    /**
    * Typedef that allows other classes to extract our template parameter.
    */
    typedef T value_type;

    /**
    * Constructs an empty locking priority queue.
    */
    PriorityQueue()
        : first(nullptr)
    {
    }

    /**
    * Pushes a task into the proper location of this queue.
    *  if the task has an equivalent priority as an existing task the new task
    *  is inserted onto the end of that chain of values.
    *
    * For now the location is found using a linear scan (ew). In the future
    *  the location will be located with binary search.
    *
    * @param task the task to push into the queue.
    * @pre None
    * @post The task is pushed into the queue.
    */
    void push(T* task)
    {
        std::unique_lock<std::mutex> lg(m);
        T** location = &first;
        while(*location != nullptr)
        {
            if(task->priority < (*location)->priority)
            {
                task->next = *location;
                break;
            }
            location = &(*location)->next;
        }

        *location = task;
        cd.notify_one();
    }

    /**
    * Attempts to retrieve step elements from this queue.
    *  This function will block until at least 1 element is ready.
    * @arg step the number of elements to try and remove.
    * @arg output a memory location to use to store a pointer to the first ready element.
    * @pre None
    * @post A linked list of exactly return value number of elements is in the output param.
    * @return the number of elements returned into the output.
    */
    size_t pop(size_t step, T** output)
    {
        std::unique_lock<std::mutex> lg(m);
        while(first == nullptr)
        {
            cd.wait(lg);
        }

        size_t gathered = 0;
        *output = first;
        T* current = first->next;
        T* output_end = first;
        while(current != nullptr && (step == 0 || gathered < step))
        {
            output_end = current;
            current = current->next;
            ++gathered;
        }

        output_end->next = nullptr;
        first = current;

        return gathered;
    }

private:
    std::mutex m;
    std::condition_variable cd;
    T* first;
};

}
