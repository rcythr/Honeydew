// This file is part of RFUS (Rich's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

#include "rfus.hpp"
#include "helpers/conditional_task.hpp"

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace rfus;

int main(int argc, char* argv[])
{
    RFUS = createRFUS(ROUND_ROBIN, 2, 1);

    std::mutex return_mut;
    std::condition_variable cv;

    {
        std::unique_lock<std::mutex> lg(return_mut);
        bool completed = false;

        RFUS->post(ConditionalTask(RFUS, [] () {
            return true;
        }).on_true(Task([&] () {
            printf("1 TRUE!\n");
            {
                std::unique_lock<std::mutex> lg(return_mut);
                completed = true;
            }
            cv.notify_all();
        })).on_false(Task([&] () {
            printf("1 FALSE!\n");
            {
                std::unique_lock<std::mutex> lg(return_mut);
                completed = true;
            }
            cv.notify_all();
        })));

        if(!completed)
            cv.wait(lg);
    }

    {
        std::unique_lock<std::mutex> lg(return_mut);
        bool completed = false;
        
        RFUS->post(ConditionalTask(RFUS, [] () {
            return false;
        }).on_true(Task([&] () {
            printf("2 TRUE\n");
            {
                std::unique_lock<std::mutex> lg(return_mut);
                completed = true;
            }
            cv.notify_all();
        })).on_false(Task([&] () {
            printf("2 FALSE!\n");
            {
                std::unique_lock<std::mutex> lg(return_mut);
                completed = true;
            }
            cv.notify_all();
        })));

        if(!completed)
            cv.wait(lg);
    }
}
