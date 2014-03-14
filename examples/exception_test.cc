// This file is part of Honeydew
// Honeydew is licensed under the MIT LICENSE. See the LICENSE file for more info.

/**
* This example shows the typical syntax and usage of the ExeceptionTask
* (helpers/exception_task.hpp) helper class.
*/

#include <honeydew/honeydew.hpp>
#include <honeydew/helpers/exception_task.hpp>

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace honeydew;

int main(int argc, char* argv[])
{
    // These variables are used to control the flow of the main thread
    //   as the event system processes the events. It is not always necessary.
    std::mutex return_mut;
    std::condition_variable cv;

    // It is always necessary to create a Honeydew when using this library.
    //   however it need not be placed into the Honeydew global variable.
    // In this case a ROUND_ROBIN honeydew is created with 2 workers which grab
    //   events one at a time.
    Honeydew* HONEYDEW = Honeydew::create(Honeydew::ROUND_ROBIN, 2, 1);

    // This case shows what happens when an exception is thrown inside
    //   the execution of an ExecutionTask wrapper.
    // Output: 1 EXCEPTION!
    {
        std::unique_lock<std::mutex> lg(return_mut);
        bool completed = false;

        // Post an Exception wrapped task which throws an exception "SOME EXCEPTION"
        HONEYDEW->post(ExceptionTask<std::runtime_error>(HONEYDEW, [] () {
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
        HONEYDEW->post(ExceptionTask<std::runtime_error>(HONEYDEW, [] () {
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
