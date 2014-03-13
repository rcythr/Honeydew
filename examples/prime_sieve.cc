// This file is part of RFUS (Rich's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

/**
* This example uses a RFUS to calculate all the prime numbers up to a given number.
*   It should be noted that this is not the most efficient way to implement a sieve
*   (Every number's multiple is used instead of the next prime remaining after each pass)
*   but it allows for some concurrency to do it this way.
*/

#include <rfus/rfus.hpp>
#include <rfus/helpers/task_wrapper.hpp>
#include <rfus/helpers/post_and_wait.hpp>

#include <iostream>
#include <thread>
#include <atomic>

using namespace rfus;

/*
* This file is a practical example of RFUS in use.
*/
int main(int argc, char* argv[])
{
    printf("Maximum Number: ");

    // Construct a sieve of the proper size
    std::string buf;
    std::getline(std::cin, buf);

    size_t sieve_size = std::stoi(buf);
    std::atomic<size_t>* sieve = new std::atomic<size_t>[sieve_size-2];

    printf("Constructing sieve of elements up to %lu...\n", sieve_size);

    // Fill it with the initial values.
    for(size_t i=2; i < sieve_size; ++i)
    {
        sieve[i-2].store(i);
    }

    // Initialize the RFUS
    RFUS = createRFUS(ROUND_ROBIN, std::thread::hardware_concurrency(), 0);

    // Schedule the tasks
    Task task([] () {});
    size_t maximum_in_range = sieve_size/2;
    for(size_t i=0; i < maximum_in_range-2; ++i)
    {
        task.also([&, i] () {
            size_t n = i+2;
            for(size_t j=n-2 + n; j < sieve_size; j += n)
            {
                sieve[j].store(0);
            }
        });
    }

    // Schedule the last printing task
    task.then([&] () {
        for(size_t i=0; i < sieve_size-2; ++i)
        {
            size_t val = sieve[i].load();
            if(val != 0)
                printf("%lu ", val);
        }
        printf("\n");
    });

    // Post to the RFUS.
    post_and_wait(RFUS, task);

    return 0;
}
