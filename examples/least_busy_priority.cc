// This file is part of RFUS (Rich's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

#include "rfus.hpp"
#include "helpers/task_wrapper.hpp"

#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>

using namespace rfus;

// In this test the program will run tasks A-F concurrently assigning them in a RR
// fashion. Afterwards it will run task G. Task G will signal the condition variable
// on the main thread which will then exit, ending the program.
int main(int argc, char* argv[])
{
    RFUS = createRFUS(LEAST_BUSY_WITH_PRIORITY, 3, 1);

    std::mutex return_mut;
    std::condition_variable cv;

    auto duration = std::chrono::system_clock::now().time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    
    std::unique_lock<std::mutex> lg(return_mut);

    RFUS->post(Task([] () {
        std::cout << std::this_thread::get_id() << " A" << std::endl;
    }, millis).also([] () {
        std::cout << std::this_thread::get_id() << " B" << std::endl;
    }, 30000).also([] () {
        std::cout << std::this_thread::get_id() << " C" << std::endl;
    }, 2).also([] () {
        std::cout << std::this_thread::get_id() << " D" << std::endl;
    }, 0).also([] () {
        std::cout << std::this_thread::get_id() << " E" << std::endl;
    }, 5).also([] () {
        std::cout << std::this_thread::get_id() << " F" << std::endl;
    }).then([&] () {
        std::cout << std::this_thread::get_id() << " G" << std::endl;
        cv.notify_all();
    }));

    cv.wait(lg);

}
