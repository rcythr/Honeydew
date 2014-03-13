// This file is part of RFUS (Rich's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

/*
* In this test the program will run tasks A-F concurrently assigning them in a LEAST_BUSY
* fashion. Afterwards it will run task G. Task G will signal the condition variable
* on the main thread which will then exit, ending the program.
*/

#include "rfus.hpp"
#include "helpers/task_wrapper.hpp"

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace rfus;

int main(int argc, char* argv[])
{
    // These variables are used to control the flow of the main thread
    //   as the event system processes the events. It is not always necessary.
    std::mutex return_mut;
    std::condition_variable cv;

    // It is always necessary to create a RFUS when using this library.
    //   however it need not be placed into the RFUS global variable.
    // In this case a LEAST_BUSY rfus is created with 3 workers which grab
    //   events two at a time.
    RFUS = createRFUS(LEAST_BUSY, 3, 2);

    std::unique_lock<std::mutex> lg(return_mut);
    bool completed = false;

    /**
    *  All of the following tasks are pushed onto the least busy queue at
    *   the time the task is posted. This is not a strict guarentee since tasks
    *   are executing and being pushed often.
    *
    *  Output:
    *       [A-F on undefined threads in undefined order]
    *       [G on undefined thread]
    */
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
        
        // Notify main thread to continue.
        {
            std::unique_lock<std::mutex> lg(return_mut);
            completed = true;
        }
        cv.notify_all();
    }));

    // Wait for G task to execute.
    while(!completed)
        cv.wait(lg);

}
