
#include "helpers/post_and_wait.hpp"

#include <mutex>
#include <condition_variable>

void rfus::post_and_wait(RFUSInterface* rfus, Task& task)
{
    std::mutex mut;
    std::condition_variable cv;
    bool complete = false;

    rfus->post(task.then([&] () {
        {
            std::unique_lock<std::mutex> lg(mut);
            complete = true;
        }
        cv.notify_all();
    }));

    std::unique_lock<std::mutex> lg(mut);
    while(!complete)
        cv.wait(lg);    
}
