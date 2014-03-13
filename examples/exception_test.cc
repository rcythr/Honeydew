// This file is part of RFUS (Rich's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

/**
* This example shows the typical syntax and usage of the ExeceptionTask
* (helpers/exception_task.hpp) helper class.
*/

#include "rfus.hpp"
#include "helpers/exception_task.hpp"

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
    // In this case a ROUND_ROBIN rfus is created with 2 workers which grab
    //   events one at a time.
    RFUS = createRFUS(ROUND_ROBIN, 2, 1);

    // This case shows what happens when an exception is thrown inside
    //   the execution of an ExecutionTask wrapper.
    // Output: 1 EXCEPTION!
    {
        std::unique_lock<std::mutex> lg(return_mut);
        bool completed = false;

        // Post an Exception wrapped task which throws an exception "SOME EXCEPTION"
        RFUS->post(ExceptionTask<std::runtime_error>(RFUS, [] () {
            throw std::runtime_error("SOME EXCEPTION");
        }).on_success(Task([&] () {
            // Print something to show which case was hit.
            printf("1 NO EXCEPTION!\n");
            
            // Notify the main thread to continue.
            {
                std::unique_lock<std::mutex> lg(return_mut);
                completed = true;
            }
            cv.notify_all();
        })).on_failure([&] (std::runtime_error& e) {
            // Print something to show which case was hit.
            printf("1 EXCEPTION!\n");
            
            // Notify the main thread to continue.
            {
                std::unique_lock<std::mutex> lg(return_mut);
                completed = true;
            }
            cv.notify_all();
        }));

        // Main thread waits here for something to happen.
        if(!completed)
            cv.wait(lg);
    }

    // This case shows the behavior when an exception is not thrown from
    //   an ExceptionTask wrapped Task.
    // Output: 2 NO EXCEPTION!
    {
        std::unique_lock<std::mutex> lg(return_mut);
        bool completed = false;
       
        // Post an ExceptionTask which does not throw an exception. 
        RFUS->post(ExceptionTask<std::runtime_error>(RFUS, [] () {
        }).on_success(Task([&] () {
            // Print something to show which case was hit.
            printf("2 NO EXCEPTION!\n");
            
            // Notify the main thread to continue.
            {
                std::unique_lock<std::mutex> lg(return_mut);
                completed = true;
            }
            cv.notify_all();
        })).on_failure([&] (std::runtime_error& e) {
            // Print something to show which case was hit.
            printf("2 EXCEPTION!\n");
            
            // Notify the main thread to continue.
            {
                std::unique_lock<std::mutex> lg(return_mut);
                completed = true;
            }
            cv.notify_all();
        }));

        // Main thread waits here for something to happen.
        if(!completed)
            cv.wait(lg);
    }

    return 0;
}
