// This file is part of Honeydew
// Honeydew is licensed under the MIT LICENSE. See the LICENSE file for more info.

#pragma once

#include <mutex>
#include <condition_variable>

namespace honeydew
{

/**
* A simple Lock based queue which can hold T elements.
*/
template<typename T>
class Queue
{
public:
    /**
    * Typedef that allows other classes to extract our template parameter.
    */
    typedef T value_type;

    /**
    * Constructs an empty locking queue.
    */
    Queue()
        : first(nullptr)
        , last(nullptr)
    {
    }

    /**
    * Pushes a task onto the end of the queue.
    * @param task the task to push onto the queue.
    * @pre None
    * @post The task is pushed onto the queue.
    */
    void push(T* task)
    {
        {
            std::unique_lock<std::mutex> lg(m);
            if(first == nullptr)
            {
                first = last = task;
            }
            else
            {
                last->next = task;
                last = task;
            }
        }
        cd.notify_all();
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

        if(first == nullptr)
            last = nullptr;

        return gathered;
    }

private:
    std::mutex m;
    std::condition_variable cd;
    T* first;
    T* last;
};

}
