// This file is part of Honeydew 
// Honeydew is licensed under the MIT LICENSE. See the LICENSE file for more info.

#include <honeydew/helpers/post_and_wait.hpp>

#include <mutex>
#include <condition_variable>

void honeydew::post_and_wait(Honeydew* honeydew, Task& task)
{
    std::mutex mut;
    std::condition_variable cv;
    bool complete = false;

    honeydew->post(task.then([&] () {
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
