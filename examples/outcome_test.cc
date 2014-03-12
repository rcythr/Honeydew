// This file is part of RFUS (Rich's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

#include "rfus.hpp"
#include "helpers/outcome_task.hpp"

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

        RFUS->post(OutcomeTask(RFUS, [] () {
            throw std::runtime_error("SOME EXCEPTION");
        }).on_success(Task([&] () {
            printf("1 NO EXCEPTION!\n");
            {
                std::unique_lock<std::mutex> lg(return_mut);
                completed = true;
            }
            cv.notify_all();
        })).on_failure(Task([&] () {
            printf("1 EXCEPTION!\n");
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
        
        RFUS->post(OutcomeTask(RFUS, [] () {
        }).on_success(Task([&] () {
            printf("2 NO EXCEPTION!\n");
            {
                std::unique_lock<std::mutex> lg(return_mut);
                completed = true;
            }
            cv.notify_all();
        })).on_failure(Task([&] () {
            printf("2 EXCEPTION!\n");
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
