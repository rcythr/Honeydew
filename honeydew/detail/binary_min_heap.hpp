// This file is part of Honeydew
// Honeydew is licensed under the MIT LICENSE. See the LICENSE file for more info.

#pragma once

#include <mutex>
#include <condition_variable>

namespace honeydew
{

/**
* A datastructure for a locking binary min-heap.
*/
template<typename T>
class BinaryMinHeap
{
public:

    typedef T value_type;

    /**
    * Constructs a new binary heap with the given initial_capacity 
    *   as the starting size of the underlying array.
    * @arg initial_capacity the initial size of the underlying array.
    */
    BinaryMinHeap(size_t initial_capacity=16)
        : size(0)
        , capacity(initial_capacity)
        , heap(new T*[initial_capacity])
    {
        
    }

    /**
    * Inserts a new task into this min heap.
    * @arg task the task to insert into the heap, ordered by ->priority.
    */
    void push(T* task)
    {
        {
            std::unique_lock<std::mutex> lg(m);

            // Make sure we're big enough for this element.
            grow();

            // Insert ourselves into the index beyond the last used.
            heap[size] = task;

            // Use sift upon on that index to correct the heap property.
            siftUp(size);

            // Increase the size to reflect the new size of the heap.
            ++size;
        }
        cd.notify_one();
    }

    /**
    * Removes up to step elements from this min-heap. If none are available
    *   this method blocks until at least one element is ready.
    * @arg step the maximum number of elements to remove.
    * @arg output a memory location for where to store the output list of tasks.
    * @return the number of tasks gathered.
    */
    size_t pop(size_t step, T** output)
    {
        std::unique_lock<std::mutex> lg(m);

        // Block until we have at least one task to return.
        while(size == 0)
        {
            cd.wait(lg);
        }

        size_t gathered = 0;
        *output = nullptr;
        T* output_end = nullptr;
        while((step == 0 || gathered < step) && size != 0)
        {
            if(*output == nullptr)
            {
                *output = heap[0];
                output_end = *output;
            }
            else
            {
                output_end->next = heap[0];
                output_end = output_end->next;
            }
            
            // Increment the count of gathered elements.
            ++gathered;

            // Decrement size to reflect new size
            --size;

            // Put the last element into the first slot.
            if(size > 0)
            {
                // Move the element from the last filled slot into the root
                heap[0] = heap[size];
                
                if(size > 1)
                {
                    // Sift down from the new root element to fix heap property
                    siftDown(0);
                }
            }

        }

        // Ensure the next of the end is pointing to nullptr.
        output_end->next = nullptr;

        return gathered;
    }

private:

    inline static size_t parent_index(size_t index) { return (index - 1) / 2; }
    inline static size_t first_index(size_t index) { return 2*index + 1; }
    inline static size_t second_index(size_t index) { return 2*index + 2; }

    inline void swap(size_t indexA, size_t indexB)
    {
        T* swap_space;
        swap_space = heap[indexA];
        heap[indexA] = heap[indexB];
        heap[indexB] = swap_space;
    }

    void grow()
    {
        if(size == capacity)
        {
            // Create the new heap
            size_t new_capacity = capacity * 2;
            T** new_heap = new T*[new_capacity];

            // Copy over data in the current heap.
            for(size_t i=0; i < size; ++i)
            {
                new_heap[i] = heap[i];
            }    

            // Perform the swap.
            delete heap;
            heap = new_heap;
            capacity = new_capacity;
        }
    }

    void siftUp(size_t index)
    {
        if(index == 0) return;

        size_t parent = parent_index(index);

        // If our parent has a higher priority than us
        // We want to swap with him and then recurse on our new location.
        if(heap[index]->priority < heap[parent]->priority)
        {
           swap(index, parent);
           siftUp(parent); 
        }
    }

    void siftDown(size_t index)
    {
        size_t first = first_index(index);
        size_t second = first + 1;
        size_t smallest = index;

        if(first < size && heap[first]->priority < heap[smallest]->priority)
            smallest = first;

        if(second < size && heap[second]->priority < heap[smallest]->priority)
            smallest = second;
        
        if(smallest != index)
        {
            swap(smallest, index);
            siftDown(smallest);
        }
    }

    size_t size;
    size_t capacity;
    T** heap;

    std::mutex m;
    std::condition_variable cd;
};

}
