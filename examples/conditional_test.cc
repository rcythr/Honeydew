// This file is part of Honeydew
// Honeydew is licensed under the MIT LICENSE. See the LICENSE file for more info.

/**
* This example shows the typical usage of the ConditionalTask 
*   (helpers/conditional_task.hpp) helper class.
*/

#include <honeydew/honeydew.hpp>
#include <honeydew/helpers/conditional_task.hpp>

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

    // Here is the first test that returns true into the ConditionalTask
    // Output: 1 TRUE!
    {
        std::unique_lock<std::mutex> lg(return_mut);
        bool completed = false;
        
        // Create a new condtional task which has a functor which returns bool.
        //   This value will determine ifthe on_true Task or on_false task is executed.
        HONEYDEW->post(ConditionalTask(HONEYDEW, [] () {
            return true;
        }).on_true(Task([&] () {
            // Print out something to show we executed this task.
            printf("1 TRUE!\n");
            
            // Notify main thread to continue.
            {
                std::unique_lock<std::mutex> lg(return_mut);
                completed = true;
            }
            cv.notify_all();
        })).on_false(Task([&] () {
            // Print out something to show we executed this task.
            printf("1 FALSE!\n");

            // Notify main thread to continue.
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

    // Here is the second test that returns false into the ConditionalTask
    // Output: 2 FALSE!
    {
        std::unique_lock<std::mutex> lg(return_mut);
        bool completed = false;
        
        // Create a new condtional task which has a functor which returns bool.
        //   This value will determine ifthe on_true Task or on_false task is executed.
        HONEYDEW->post(ConditionalTask(HONEYDEW, [] () {
            return false;
        }).on_true(Task([&] () {
            // Print out something to show we executed this task.
            printf("2 TRUE\n");

            // Notify main thread to continue.
            {
                std::unique_lock<std::mutex> lg(return_mut);
                completed = true;
            }
            cv.notify_all();
        })).on_false(Task([&] () {
            // Print out something to show we executed this task.
            printf("2 FALSE!\n");
        
            // Notify main thread to continue.
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
