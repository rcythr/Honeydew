// This file is part of Honeydew
// Honeydew is licensed under the MIT LICENSE. See the LICENSE file for more info.

/** 
* In this test the program will run tasks A-F concurrently assigning them in a LEAST_BUSY
*   fashion. Afterwards it will run task G. Task G will signal the condition variable
*   on the main thread which will then exit, ending the program.
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
    // In this case a LEAST_BUSY_PRIORITY honeydew is created with 3 workers which grab
    //   events one at a time.
    Honeydew* HONEYDEW = Honeydew::create(Honeydew::LEAST_BUSY_WITH_PRIORITY, 3, 1);

    /* 
    * All of the following tasks are pushed onto thread 1. With a decreasing
    *   priority. Because the internal thread does not block for all elements
    *   below to be inserted the priority is not strictly enforced and the output
    *   varies.
    * Output:
    *          [A-F in undefined order which prefers tasks in order from F->A]]
    *          G
    */
    post_and_wait(HONEYDEW, Task([] () {
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
    }));

    return 0;
}
