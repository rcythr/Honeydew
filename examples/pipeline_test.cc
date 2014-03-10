// This file is part of RFUS (Rcythr's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

#include "rfus.h"
#include "pipeline.h"

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

int main(int argc, char* argv[])
{
    RFUS = createRFUS(ROUND_ROBIN, 2, 1);

    std::mutex return_mut;
    std::condition_variable cv;

    std::unique_lock<std::mutex> lg(return_mut);

    RFUS.post(Pipeline::start([] () -> int { return 5; }).then([] (int val) { printf("%d\n", val); }).close());

    cv.wait(lg);

}
