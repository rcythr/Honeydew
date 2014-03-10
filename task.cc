// This file is part of RFUS (Rcythr's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

#include "task.h"
#include "join_semaphore.h"

task_t::task_t(std::function<void()> action, size_t worker, uint64_t deadline)
    : action(action)
    , priority(deadline)
    , continuation(nullptr)
    , join(nullptr)
    , worker(worker)
    , next(nullptr)
{
}

Task::~Task()
{
    // If task was never extracted.
    // Delete it and component objects.
    task_t* current = root;
    task_t* prev;
    join_semaphore_t* prev_join = nullptr;
    while(current != nullptr)
    {
        if(current->join == nullptr && prev_join != nullptr)
        {
            delete prev_join;
            prev_join = nullptr;
        }
        prev_join = current->join;
        prev = current;
        if(current->next != nullptr)
            current = current->next;
        else
            current = current->continuation;
        delete prev;
    }

    if(prev_join != nullptr)
        delete prev_join;
}

Task::Task(std::function<void()> action)
    : root(nullptr)
    , or_root(nullptr)
    , leaf(nullptr)
{
    root = leaf = new task_t(action, 0, 0);
}

Task::Task(std::function<void()> action, uint64_t deadline)
{
    root = leaf = new task_t(action, 0, deadline);
}

Task::Task(size_t worker, std::function<void()> action)
{
    root = leaf = new task_t(action, worker, 0);
}

Task::Task(size_t worker, std::function<void()> action, uint64_t deadline)
{
    root = leaf = new task_t(action, worker, deadline);
}

Task& Task::then(std::function<void()> action)
{
    leaf = (leaf->continuation = new task_t(action, 0, 0));

    while(or_root != nullptr)
    {
        or_root->continuation = leaf;
        or_root = or_root->next;
    }

    return *this;
}

Task& Task::then(std::function<void()> action, uint64_t deadline)
{
    leaf = leaf->continuation = new task_t(action, 0, leaf->priority + deadline);
    while(or_root != nullptr)
    {
        or_root->continuation = leaf;
        or_root = or_root->next;
    }
    return *this;
}

Task& Task::thenAbsolute(std::function<void()> action, uint64_t deadline)
{
    leaf = leaf->continuation = new task_t(action, 0, deadline);
    while(or_root != nullptr)
    {
        or_root->continuation = leaf;
        or_root = or_root->next;
    }
    return *this;
}

Task& Task::then(size_t worker, std::function<void()> action)
{
    leaf = leaf->continuation = new task_t(action, worker, 0);
    while(or_root != nullptr)
    {
        or_root->continuation = leaf;
        or_root = or_root->next;
    }
    return *this;
}

Task& Task::then(size_t worker, std::function<void()> action, uint64_t deadline)
{
    leaf = leaf->continuation = new task_t(action, worker, leaf->priority + deadline);
    while(or_root != nullptr)
    {
        or_root->continuation = leaf;
        or_root = or_root->next;
    }
    return *this;
}

Task& Task::thenAbsolute(size_t worker, std::function<void()> action, uint64_t deadline)
{
    leaf = leaf->continuation = new task_t(action, worker, deadline);
    while(or_root != nullptr)
    {
        or_root->continuation = leaf;
        or_root = or_root->next;
    }
    return *this;
}

Task& Task::also(std::function<void()> action)
{
    join_semaphore_t* join;
    if(leaf->join == nullptr)
    {
        or_root = leaf;
        join = leaf->join = new join_semaphore_t(2);
    }
    else
    {
        join = leaf->join;
        join->increment();
    }
    leaf = leaf->next = new task_t(action, 0, 0);
    leaf->join = join;
    return *this;
}

Task& Task::also(std::function<void()> action, uint64_t deadline)
{
    join_semaphore_t* join;
    if(leaf->join == nullptr)
    {
        or_root = leaf;
        join = leaf->join = new join_semaphore_t(2);
    }
    else
    {
        join = leaf->join;
        join->increment();
    }
    leaf = leaf->next = new task_t(action, 0, leaf->priority + deadline);
    leaf->join = join;
    return *this;
}

Task& Task::alsoAbsolute(std::function<void()> action, uint64_t deadline)
{
    join_semaphore_t* join;
    if(leaf->join == nullptr)
    {
        or_root = leaf;
        join = leaf->join = new join_semaphore_t(2);
    }
    else
    {
        join = leaf->join;
        join->increment();
    }
    leaf = leaf->next = new task_t(action, 0, deadline);
    leaf->join = join;
    return *this;
}

Task& Task::also(size_t worker, std::function<void()> action)
{
    join_semaphore_t* join;
    if(leaf->join == nullptr)
    {
        or_root = leaf;
        join = leaf->join = new join_semaphore_t(2);
    }
    else
    {
        join = leaf->join;
        join->increment();
    }
    leaf = leaf->next = new task_t(action, worker, 0);
    leaf->join = join;
    return *this;
}

Task& Task::also(size_t worker, std::function<void()> action, uint64_t deadline)
{
    join_semaphore_t* join;
    if(leaf->join == nullptr)
    {
        or_root = leaf;
        join = leaf->join = new join_semaphore_t(2);
    }
    else
    {
        join = leaf->join;
        join->increment();
    }
    leaf = leaf->next = new task_t(action, worker, leaf->priority + deadline);
    leaf->join = join;
    return *this;
}

Task& Task::alsoAbsolute(size_t worker, std::function<void()> action, uint64_t deadline)
{
    join_semaphore_t* join;
    if(leaf->join == nullptr)
    {
        or_root = leaf;
        join = leaf->join = new join_semaphore_t(2);
    }
    else
    {
        join = leaf->join;
        join->increment();
    }
    leaf = leaf->next = new task_t(action, worker, deadline);
    leaf->join = join;
    return *this;
}

task_t* Task::getTask()
{
    task_t* result = root;
    root = nullptr;
    return result;
}
