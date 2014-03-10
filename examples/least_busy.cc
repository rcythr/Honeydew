// This file is part of RFUS (Rcythr's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

#include "rfus.h"

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

// In this test the program will run tasks A-F concurrently assigning them in a RR
// fashion. Afterwards it will run task G. Task G will signal the condition variable
// on the main thread which will then exit, ending the program.
int main(int argc, char* argv[])
{
    RFUS = createRFUS(LEAST_BUSY, 3, 2);

    std::mutex return_mut;
    std::condition_variable cv;

    std::unique_lock<std::mutex> lg(return_mut);

    RFUS->post(Task([] () {
        std::cout << std::this_thread::get_id() << " A" << std::endl;
    }).also([] () {
        std::cout << std::this_thread::get_id() << " B" << std::endl;
    }).also([] () {
        std::cout << std::this_thread::get_id() << " C" << std::endl;
    }).also([] () {
        std::cout << std::this_thread::get_id() << " D" << std::endl;
    }).also([] () {
        std::cout << std::this_thread::get_id() << " E" << std::endl;
    }).also([] () {
        std::cout << std::this_thread::get_id() << " F" << std::endl;
    }).then([&] () {
        std::cout << std::this_thread::get_id() << " G" << std::endl;
        cv.notify_all();
    }));

    cv.wait(lg);

}
