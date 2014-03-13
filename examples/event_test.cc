#include "helpers/event_processor.hpp"

#include <mutex>
#include <condition_variable>

using namespace rfus;

struct MockEventType
{
    typedef const char cast_type;

    MockEventType(const char* val)
        : name(val)
    {
    }

    std::string name;
};

int main(int argc, char* argv[])
{
    RFUS = createRFUS(ROUND_ROBIN, 2, 1);

    EventProcessor<int> event_system(RFUS);

    std::mutex mut;
    std::condition_variable cv;
    bool complete = false;
    
    std::unique_lock<std::mutex> lg(mut);

    event_system.bind_constructable<MockEventType>(35, 
    [&](MockEventType& event)  {    
        printf("%s\n", event.name.c_str());
        {
            std::unique_lock<std::mutex> lg(mut);
            complete = true;
        }
        cv.notify_all();
    });


    char* val = "Doggy!";
    event_system.post_event(35, val);

    while(!complete)
        cv.wait(lg);

    return 0;
}
