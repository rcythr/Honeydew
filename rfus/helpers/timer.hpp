#pragma once

#include <rfus/helpers/task_wrapper.hpp>

#include <queue>
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>

#include <iostream>

namespace rfus
{
namespace detail
{

struct TimerTask
{
    TimerTask(std::function<bool()> functor, size_t worker, uint64_t priority, uint64_t period, uint64_t next_time)
        : functor(functor)
        , worker(worker)
        , priority(priority)
        , period(period)
        , next_time(next_time)
    {
    }

    std::function<bool()> functor;
    size_t worker;
    uint64_t priority;
    uint64_t period;
    uint64_t next_time;
};

struct TimerComp
{
    bool operator() (const detail::TimerTask& lhs, const detail::TimerTask& rhs) const
    {
    return lhs.next_time < rhs.next_time;
    }
};

}

/**
* A helper class that allows for the posting of tasks to a given RFUS.
*  timer_period is the time in DurationType around which the timer thread will check
*  the queue of TimerTasks.
*  DurationType is a std::chrono type (such as std::chrono::milliseconds) which defines the
*  units of the periods given to this timer.
*/
template<uint64_t timer_period, typename DurationType=std::chrono::milliseconds>
class Timer
{
public:

    /**
    * Constructs a new Timer with the given rfus.
    */
    Timer(RFUSInterface* rfus)
        : rfus(rfus)
    {
        // Initialize to 1.
        running.test_and_set();

        // Start a timer thread.
        timer_thread = std::thread(std::bind(&Timer<timer_period>::run, this));
    }

    /**
    * Deleted copy constructor & copy assignment
    */
    Timer(const Timer& other) = delete;
    Timer& operator=(const Timer& other) = delete;

    /**
    * Default move construction & assignment.
    */
    Timer(Timer&& other) = default;
    Timer& operator=(Timer&& other) = default;
   
    /**
    * Schedules a new task into this timer.
    * @arg functor the functor to run. If it returns true this task will be rescheduled at time [current_time] + period
    * @arg period the period of the task given in DurationType units.
    * @arg worker the worker thread to run the functor upon.
    * @arg priority the priority of the timed task.
    */
    void schedule(std::function<bool()> functor, uint64_t period, size_t worker=0, uint64_t priority=0)
    {
        std::unique_lock<std::mutex> lg(mut);
        queue.emplace(functor, worker, priority, period, current_time_nanos() + period);
    }

    /**
    * Attempts to shutdown this timer by setting an atomic flag
    *  and then joining on the timer thread. This method blocks
    *  until the timer thread has finished executing.
    */
    void shutdown()
    {
        running.clear();
        timer_thread.join();
    }

private:
    typedef std::priority_queue<detail::TimerTask, std::vector<detail::TimerTask>, detail::TimerComp> QueueType;
    
    static uint64_t current_time_nanos()
    {
        return std::chrono::duration_cast<DurationType>(std::chrono::system_clock::now().time_since_epoch()).count();
    }
    
    void run()
    {
        while(running.test_and_set())
        {
            {
                std::unique_lock<std::mutex> lg(mut);
                uint64_t current_time = current_time_nanos();
                
                while(!queue.empty() && queue.top().next_time <= current_time)
                {
                    detail::TimerTask top = queue.top();
                    rfus->post(Task([=]() {
                        if(top.functor())
                        {
                            // Reschedule in the timer.
                            this->schedule(top.functor, top.period, top.worker, top.priority);
                        }                        
                    }, top.worker, top.priority));
                    queue.pop();
                }
            }

            std::this_thread::sleep_for(DurationType(timer_period));
        }
    }

    RFUSInterface* rfus;
    std::thread timer_thread;
    std::atomic_flag running;

    std::mutex mut;
    QueueType queue;
};

}
