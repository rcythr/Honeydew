
#include <rfus/rfus.hpp>
#include <rfus/helpers/timer.hpp>

#include <iostream>
#include <mutex>
#include <condition_variable>

using namespace rfus;

int main(int argc, char* argv[])
{
    std::mutex mut;
    std::condition_variable cv;
    bool complete = false;

    RFUS = createRFUS(ROUND_ROBIN, 3, 1);

    Timer<100> t(RFUS); // Specified in nanoseconds.
    int counter = 0;

    std::unique_lock<std::mutex> lg(mut);

    t.schedule([&]() {
        printf("Task Tick!\n");
        if(++counter < 5)
        {
            return true;
        }
        else
        {
            {
                std::unique_lock<std::mutex> lg(mut);
                complete = true;
            }
            cv.notify_all();
            return false;
        }
    }, 1000);

    while(!complete)
        cv.wait(lg);

    t.shutdown();

    return 0;
}
