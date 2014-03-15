// This file is part of Honeydew 
// Honeydew is licensed under the MIT LICENSE. See the LICENSE file for more info.

#include <honeydew/helpers/task_wrapper.hpp>
#include <honeydew/detail/join_semaphore.hpp>

using namespace honeydew;

Task::Task()
    : root(nullptr)
    , or_root(nullptr)
    , leaf(nullptr)
{
}

void Task::init(std::function<void()> action, size_t worker, uint64_t deadline)
{
    if(root != nullptr)
        throw std::runtime_error("Cannot re-initialize task_wrapper!");

    root = leaf = new task_t(action, worker, deadline);
}

Task::Task(std::function<void()> action, size_t worker, uint64_t deadline)
    : or_root(nullptr)
{
    root = leaf = new task_t(action, worker, deadline);
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

Task& Task::operator=(Task&& other)
{
    root = other.root;
    or_root = other.or_root;
    leaf = other.leaf;

    other.root = nullptr;
    other.or_root = nullptr;
    other.leaf = nullptr;
    return *this;
}

Task::~Task()
{
    if(root == nullptr)
        delete root;
    root = nullptr;
}

Task& Task::then(std::function<void()> action, size_t worker, uint64_t deadline)
{
    return then_absolute(std::move(action), worker, leaf->priority + deadline);
}

Task& Task::then_absolute(std::function<void()> action, size_t worker, uint64_t deadline)
{
    join_semaphore_t* root_sem = nullptr;

    leaf = leaf->continuation = new task_t(action, worker, deadline);

    if(or_root != nullptr)
    {
        root_sem = or_root->join;

        while(or_root != nullptr && or_root->join == root_sem)
        {
            or_root->continuation = leaf;
            or_root = or_root->next;
        }
        or_root = nullptr;
    }
    return *this;
}

Task& Task::then(task_t* other)
{
    join_semaphore_t* root_sem = nullptr;
    leaf = leaf->continuation = other;
    
    // Fix up previous also join, if exists
    if(or_root != nullptr)
    {
        root_sem = or_root->join;

        while(or_root != nullptr && or_root->join == root_sem)
        {
            or_root->continuation = leaf;
            or_root = or_root->next;
        }
        or_root = nullptr;
    }

    // Fix up leaf pointer.
    while(leaf->continuation != nullptr)
    {
        leaf = leaf->continuation;
    }

    return *this;
}

Task& Task::also(std::function<void()> action, size_t worker, uint64_t deadline)
{
    return also_absolute(std::move(action), worker, leaf->priority + deadline);
}

Task& Task::also_absolute(std::function<void()> action, size_t worker, uint64_t deadline)
{
    join_semaphore_t* join;
    task_t* new_task = new task_t(action, worker, deadline);
    
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

    if(leaf->next != nullptr) 
    { 
        new_task->next = leaf->next; 
    }
    leaf = leaf->next = new_task;
    leaf->join = join;
    return *this;
}

Task& Task::also(task_t* other)
{
    return *this;
}

Task& Task::fork(std::function<void()> action, size_t worker, uint64_t deadline)
{
    return fork_absolute(std::move(action), worker, leaf->priority + deadline);
}

Task& Task::fork_absolute(std::function<void()> action, size_t worker, uint64_t deadline)
{
    task_t* task = new task_t(action, worker, deadline);
    task->next = leaf->next;
    leaf->next = task;
    return *this;
}

Task& Task::fork(task_t* other)
{
    task_t* task = other; 
    task->next = leaf->next;
    leaf->next = other;
    return *this;
}

task_t* Task::close()
{
    task_t* result = root;
    root = nullptr;
    return result;
}
