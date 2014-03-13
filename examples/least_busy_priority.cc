// This file is part of RFUS (Rich's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

/** 
* In this test the program will run tasks A-F concurrently assigning them in a LEAST_BUSY
*   fashion. Afterwards it will run task G. Task G will signal the condition variable
*   on the main thread which will then exit, ending the program.
*/
#include "rfus.hpp"
#include "helpers/task_wrapper.hpp"

#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
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
    // In this case a LEAST_BUSY_PRIORITY rfus is created with 3 workers which grab
    //   events one at a time.
    RFUS = createRFUS(LEAST_BUSY_WITH_PRIORITY, 3, 1);

    std::unique_lock<std::mutex> lg(return_mut);
    bool completed = false;

    /* 
    * All of the following tasks are pushed onto thread 1. With a decreasing
    *   priority. Because the internal thread does not block for all elements
    *   below to be inserted the priority is not strictly enforced and the output
    *   varies.
    * Output:
    *          [A-F in undefined order which prefers tasks in order from F->A]]
    *          G
    */
    RFUS->post(Task([] () {
        std::cout << std::this_thread::get_id() << " A" << std::endl;
    }, 1, 5).also([] () {
        std::cout << std::this_thread::get_id() << " B" << std::endl;
    }, 1, 4).also([] () {
        std::cout << std::this_thread::get_id() << " C" << std::endl;
    }, 1, 3).also([] () {
        std::cout << std::this_thread::get_id() << " D" << std::endl;
    }, 1, 2).also([] () {
        std::cout << std::this_thread::get_id() << " E" << std::endl;
    }, 1, 1).also([] () {
        std::cout << std::this_thread::get_id() << " F" << std::endl;
    }, 1, 0).then([&] () {
        std::cout << std::this_thread::get_id() << " G" << std::endl;

        // Notify main thread to continue.
        {
            std::unique_lock<std::mutex> lg(return_mut);
            completed = true;
        }
        cv.notify_all();
    }));

    // Main thread waits for completion.
    while(!completed)
        cv.wait(lg);

}
