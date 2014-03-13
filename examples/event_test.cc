/*
* This example shows the typical syntax associated with the EventProcessor 
*   (helpers/event_processor.hpp) helper class.
*/

#include "helpers/event_processor.hpp"

#include <mutex>
#include <condition_variable>

using namespace rfus;

/**
 * This little event type is used in the bind_constructable use case shown below.
 * 
 * It is a type that has a constructor which takes a pointer to the type passed 
 *   into the post_event method of the EventProcessor. It then uses this pointer
 *   for initialization purposes and deletes it if necessary.
 */
struct MockEventType
{
    /**
     * When using bind_constructable it is possible to avoid the need for an extra
     *   parameter by creating a cast_type typedef in the event class.
     */
    typedef const char cast_type;

    /**
     * This is the constructor which will be used in the EventProcessor to create 
     *   this event type.
     */
    MockEventType(const char* val)
        : name(val)
    {
    }

    std::string name;
};

int main(int argc, char* argv[])
{
    // These variables are used to control the flow of the main thread
    //   as the event system processes the events. It is not always necessary.
    std::mutex mut;
    std::condition_variable cv;
    bool complete = false;
    
    // It is always necessary to create a RFUS when using this library.
    //
    // In this case a ROUND_ROBIN rfus is created with 2 workers which grab
    //   events one at a time.
    RFUS = createRFUS(ROUND_ROBIN, 2, 1);

    // Next we create an EventProcessor using the KeyType we want and rfus we want
    EventProcessor<int> event_system(RFUS);

    // Now we bind into the event processor a single event type with handler function.
    event_system.bind_constructable<MockEventType>(35, 
    [&](MockEventType& event)  {    
        
        // Print the event data out to the console
        printf("%s\n", event.name.c_str());
        
        // Notify the main thread that this event has taken place.
        {
            std::unique_lock<std::mutex> lg(mut);
            complete = true;
        }
        cv.notify_all();
    });

    // Now we lock on the mutex that protects the "complete" boolean.    
    std::unique_lock<std::mutex> lg(mut);

    // Create the input data
    char val[] = {'D', 'O', 'G', 'G', 'Y', '\0'};
    
    // Post the event which uses the key and data we created above.
    event_system.post_event(35, val);

    // Now the main thread waits for the RFUS to process the event we just posted.
    while(!complete)
        cv.wait(lg);

    return 0;
}
