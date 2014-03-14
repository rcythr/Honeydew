// This file is part of Honeydew
// Honeydew is licensed under the MIT LICENSE. See the LICENSE file for more info.

/**
* This example shows the typical usage of the OutcomeTask 
*   (helpers/outcome_task.hpp) helper class.
*/

#include <honeydew/honeydew.hpp>
#include <honeydew/helpers/outcome_task.hpp>

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

    // In this case we see the output of an OutcomeTask wrapped Task
    //   when any exception is thrown.
    // Output: 1 EXCEPTION!
    {
        std::unique_lock<std::mutex> lg(return_mut);
        bool completed = false;

        HONEYDEW->post(OutcomeTask(HONEYDEW, [] () {
            throw std::runtime_error("SOME EXCEPTION");
        }).on_success(Task([&] () {
            // Print some output to show which case was hit. 
            printf("1 NO EXCEPTION!\n");
            
            // Notify the main thread to continue.
            {
                std::unique_lock<std::mutex> lg(return_mut);
                completed = true;
            }
            cv.notify_all();
        })).on_failure(Task([&] () {
            // Print some output to show which case was hit. 
            printf("1 EXCEPTION!\n");
            
            // Notify the main thread to continue.
            {
                std::unique_lock<std::mutex> lg(return_mut);
                completed = true;
            }
            cv.notify_all();
        })));

        // Main thread waits here for either the true or false task to be executed.
        if(!completed)
            cv.wait(lg);
    }

    // In this case we see the output of an OutcomeTask wrapped Task
    //   when no exception is thrown.
    // Output: 2 NO EXCEPTION!
    {
        std::unique_lock<std::mutex> lg(return_mut);
        bool completed = false;
        
        HONEYDEW->post(OutcomeTask(HONEYDEW, [] () {
        }).on_success(Task([&] () {
            // Print some output to show which case was hit. 
            printf("2 NO EXCEPTION!\n");
            
            // Notify the main thread to continue.
            {
                std::unique_lock<std::mutex> lg(return_mut);
                completed = true;
            }
            cv.notify_all();
        })).on_failure(Task([&] () {
            // Print some output to show which case was hit. 
            printf("2 EXCEPTION!\n");
        
            // Notify the main thread to continue.
            {
                std::unique_lock<std::mutex> lg(return_mut);
                completed = true;
            }
            cv.notify_all();
        })));
        
        // Main thread waits here for either the true or false task to be executed.
        if(!completed)
            cv.wait(lg);
    }

    return 0;
}
