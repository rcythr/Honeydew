
#include "helpers/task_wrapper.hpp"
#include "join_semaphore.hpp"

using namespace rfus;

Task::Task()
    : root(nullptr)
    , leaf(nullptr)
    , or_root(nullptr)
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

task_t* Task::close()
{
    task_t* result = root;
    root = nullptr;
    return result;
}
