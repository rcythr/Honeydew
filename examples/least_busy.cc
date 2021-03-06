// This file is part of Honeydew
// Honeydew is licensed under the MIT LICENSE. See the LICENSE file for more info.

/*
* In this test the program will run tasks A-F concurrently assigning them in a LEAST_BUSY
* fashion. Afterwards it will run task G. Task G will signal the condition variable
* on the main thread which will then exit, ending the program.
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
    // In this case a LEAST_BUSY honeydew is created with 3 workers which grab
    //   events two at a time.
    Honeydew* HONEYDEW = Honeydew::create(Honeydew::LEAST_BUSY, 3, 2);

    /**
    *  All of the following tasks are pushed onto the least busy queue at
    *   the time the task is posted. This is not a strict guarentee since tasks
    *   are executing and being pushed often.
    *
    *  Output:
    *       [A-F on undefined threads in undefined order]
    *       [G on undefined thread]
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

}
