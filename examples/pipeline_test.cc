// This file is part of RFUS (Rich's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

/**
*  This file shows some possible use cases for the Pipeline helper class
*    (helpers/pipeline.hpp). 
*
*   Warning: This class is very complex. Reading the documentation on the wiki
*     is vital to understanding its behavior.
*/

#include "rfus.hpp"
#include "helpers/pipeline.hpp"

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
    bool complete = false;

    RFUS = createRFUS(ROUND_ROBIN, 2, 1);

    std::unique_lock<std::mutex> lg(return_mut);
    complete = false;

    // This pipeline shows the behavior when a pipeline is closed prematurely.
    //   Note: the trailing 'return true' is completely ignored.
    // Output:
    //         5
    //         'a'
    RFUS->post(Pipeline::start<int>([] () { 
        return 5; 
    }).then<char>([] (int val) { 
        printf("%d\n", val); 
        return 'a'; 
    }).close_with<bool>([&] (char val) { 
        printf("%c\n", val); 

        // Notify main thread to continue
        {
            std::unique_lock<std::mutex> lg(return_mut);
            complete = true;
        }
        cv.notify_all();
        // End Notify
    
        return true; 
    }));

    while(!complete)
        cv.wait(lg);

    // Reset the complete variable.
    complete = false;

    // This pipeline shows the behavior when a call to split() is used.
    // Output:
    //        42
    //        42
    //        46
    RFUS->post(Pipeline::start<int>([] () {
        return 42;
    }).split<int>([] (int val) { // In: 42, Out: 45.
        return val+3;
    }).also<void>([] (int val) { // In 42
        printf("%d\n", val);
    }).also<int>([] (int val) {  // In 42, Out 45 (discarded)
        return val+3;
    }).join<int>([] (int val) {  // In 42, Out 46
        printf("%d\n", val);
        return val+4;
    }).then([&] (int val) {      // In 46
        printf("%d\n", val);
    
        // Notify main thread to continue
        {
            std::unique_lock<std::mutex> lg(return_mut);
            complete = true;
        }
        cv.notify_all();
    })); 

    while(!complete)
        cv.wait(lg);
}
