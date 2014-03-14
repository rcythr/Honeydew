// This file is part of Honeydew
// Honeydew is licensed under the MIT LICENSE. See the LICENSE file for more info.

/*
* In this test the program will run tasks A-F concurrently assigning them in a RR
*   fashion. Afterwards it will run task G. Task G will signal the condition variable
*    on the main thread which will then exit, ending the program.
*/

#include <honeydew/honeydew.hpp>
#include <honeydew/helpers/task_wrapper.hpp>
#include <honeydew/helpers/post_and_wait.hpp>

#include <iostream>
#include <thread>

using namespace honeydew;

int main(int argc, char* argv[])
{
    // It is always necessary to create a Honeydew when using this library.
    //   however it need not be placed into the Honeydew global variable.
    // In this case a LEAST_BUSY_PRIORITY honeydew is created with 2 workers which grab
    //   events one at a time.
    Honeydew* HONEYDEW = Honeydew::create(Honeydew::ROUND_ROBIN, 2, 1);

    /*
    * Posts tasks A-F first onto queues in a round robin fashion.
    *   After these tasks are completed Task G will be pushed onto
    *   the next queue in the RR chain.
    * Output:
    *    [A-F in undefined order with A,C,E happning from thread 1, and B, D, F happening from thread 2.]
    *    G happening from thread 1.
    */
    post_and_wait(HONEYDEW, Task([] () {
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
    }));

    return 0;
}
