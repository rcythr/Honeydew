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
    leaf = leaf->continuation = new task_t(action, worker, deadline);
    while(or_root != nullptr)
    {
        or_root->continuation = leaf;
        or_root = or_root->next;
    }
    return *this;
}

task_t* Task::then_close(task_t* other)
{
    leaf = leaf->continuation = other;
    while(or_root != nullptr)
    {
        or_root->continuation = leaf;
        or_root = or_root->next;
    }
    return this->close();
}

Task& Task::also(std::function<void()> action, size_t worker, uint64_t deadline)
{
    return also_absolute(std::move(action), worker, leaf->priority + deadline);
}

Task& Task::also_absolute(std::function<void()> action, size_t worker, uint64_t deadline)
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

task_t* Task::also_close(task_t* other)
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
    leaf = leaf->next = other; 
    leaf->join = join;
    return this->close();
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
