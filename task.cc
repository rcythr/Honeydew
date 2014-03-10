// This file is part of RFUS (Rich's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

#include "task.hpp"
#include "join_semaphore.hpp"

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

Task::Task(Task&& other)
    : root(other.root)
    , or_root(other.or_root)
    , leaf(other.leaf)
{
    other.root = nullptr;
    other.or_root = nullptr;
    other.leaf = nullptr;
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

Task::Task(std::function<void()> action, size_t worker, uint64_t deadline)
    : or_root(nullptr)
{
    root = leaf = new task_t(action, worker, deadline);
}

Task& Task::then(std::function<void()> action, size_t worker, uint64_t deadline)
{
    leaf = leaf->continuation = new task_t(action, worker, leaf->priority + deadline);
    while(or_root != nullptr)
    {
        or_root->continuation = leaf;
        or_root = or_root->next;
    }
    return *this;
}

Task& Task::thenAbsolute(std::function<void()> action, size_t worker, uint64_t deadline)
{
    leaf = leaf->continuation = new task_t(action, worker, deadline);
    while(or_root != nullptr)
    {
        or_root->continuation = leaf;
        or_root = or_root->next;
    }
    return *this;
}

Task& Task::also(std::function<void()> action, size_t worker, uint64_t deadline)
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

Task& Task::alsoAbsolute(std::function<void()> action, size_t worker, uint64_t deadline)
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

Task& Task::fork(std::function<void()> action, size_t worker, uint64_t deadline)
{
    task_t* task = new task_t(action, worker, deadline);
    task->next = leaf->next;
    leaf->next = task;
    return *this;
}

Task& Task::forkAbsolute(std::function<void()> action, size_t worker, uint64_t deadline)
{
    task_t* task = new task_t(action, worker, leaf->priority + deadline);
    task->next = leaf->next;
    leaf->next = task;
    return *this;
}

task_t* Task::getTask()
{
    task_t* result = root;
    root = nullptr;
    return result;
}
