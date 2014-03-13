// This file is part of RFUS (Rich's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

/*
* In this test the program will run tasks A-F concurrently assigning them in a RR
* fashion and reorder tasks based on priority. Afterwards it will run task G. 
* Task G will signal the condition variable on the main thread which will then exit.
*/

#include <rfus/rfus.hpp>
#include <rfus/helpers/task_wrapper.hpp>
#include <rfus/helpers/post_and_wait.hpp>

#include <iostream>
#include <thread>

using namespace rfus;

int main(int argc, char* argv[])
{
    // It is always necessary to create a RFUS when using this library.
    //   however it need not be placed into the RFUS global variable.
    // In this case a ROUND_ROBIN_WITH_PRIORITY rfus is created with 2 workers which grab
    //   events one at a time.
    RFUS = createRFUS(ROUND_ROBIN_WITH_PRIORITY, 2, 1);
    
    /*
    * Posts tasks A-F first onto queues in a round robin fashion.
    *   After these tasks are completed Task G will be pushed onto
    *   the next queue in the RR chain. Note that repriotization may happen
    *   hapen on the given tasks, but may not happen in time before the worker processes
    *   the next available task. This makes priority non-strict.
    * Output:
    *    [A-F in undefined order with A,C,E happning from thread 1, and B, D, F happening from thread 2.]
    *    G happening from thread 1.
    */
    post_and_wait(RFUS, Task([] () {
        std::cout << std::this_thread::get_id() << " A" << std::endl;
    }, 0, 5).also([] () {
        std::cout << std::this_thread::get_id() << " B" << std::endl;
    }, 0, 4).also([] () {
        std::cout << std::this_thread::get_id() << " C" << std::endl;
    }, 0, 3).also([] () {
        std::cout << std::this_thread::get_id() << " D" << std::endl;
    }, 0, 2).also([] () {
        std::cout << std::this_thread::get_id() << " E" << std::endl;
    }, 0, 1).also([] () {
        std::cout << std::this_thread::get_id() << " F" << std::endl;
    }, 0, 0).then([&] () {
        std::cout << std::this_thread::get_id() << " G" << std::endl;
    }));

    return 0;
}
