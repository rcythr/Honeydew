// This file is part of RFUS (Rich's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

#pragma once

#include "rfus.hpp"
#include "helpers/task_wrapper.hpp"
#include "helpers/pipeline.hpp"

#include <unordered_map>

namespace rfus
{

namespace detail
{

struct GenericFunction
{        
    std::function<void(void* data)> func;
    size_t worker;
    uint64_t priority;
};

}

/**
* A class which uses a RFUS to do dispatching of events.
*  Different events can be setup to dispatch to different threads as needed.
*/
template<typename KeyType>
struct EventProcessor
{
public:

    /**
    * Constructs a new EventProcessor which will run on the given RFUS.
    */
    EventProcessor(RFUSInterface* rfus)
        : rfus(rfus)
    {
    }

    /**
    * Binds an event into this dispatch which involves 3 steps:
    *  1. Casting of the input event to CastType*
    *  2. Construction of an EventDataType object from the CastType*.
    *  3. Dispatch of the EventDataType& to a given handler function.
    * If the given handler_worker is different from the given construction_worker the construction
    *  and handler will happen on different workers with the respective priorities. 
    * If the given handler_worker is the same as the given construction_worker the construction 
    *  and handler will happen on the same thread with handler_priority and construction_priority is ignored.
    * @arg key_value the value of the key for this event.
    * @arg handler the function used to handle the EventDataType& object created.
    * @arg handler_worker the worker to run the handler upon.
    * @arg handler_priority the priority of the handler task.
    * @arg construction_worker the worker used to construct the EventDataType upon.
    * @arg construction_priority the priority of the construction task.
    * @return a reference to this object for daisy chains
    */
    template<typename EventDataType, typename CastType=typename EventDataType::cast_type>
    EventProcessor& bind_constructable(KeyType key_value, std::function<void(EventDataType&)> handler, 
                                       size_t handler_worker=0, uint64_t handler_priority=0,
                                       size_t construction_worker=0, uint64_t construction_priority=0) 
    {
        RFUSInterface*& rfus_copy = rfus;
        if(construction_worker == handler_worker)
        {
            event_handlers.emplace(
                key_value,
                detail::GenericFunction{[=](void* data) {
                    EventDataType event(static_cast<CastType*>(data));
                    handler(event);
                }, handler_worker, handler_priority}
            );
        }
        else
        {
            event_handlers.emplace(
                key_value,
                detail::GenericFunction{[=](void* data) {
                    auto event = new EventDataType(static_cast<CastType*>(data));
                    rfus_copy->post(Task([=] () {
                        handler(*event);
                        delete event;
                    }, handler_worker, handler_priority));
                }, construction_worker, construction_priority}
            );
        }
        return *this;
    }

    /**
    * Constructs a new event handler which involves the following steps:
    *  1. Casting of the event data to CastType*.
    *  2. Passing of the casted event data to the given handler.
    * @arg key_value the value of the event key
    * @arg functor the handler function.
    * @arg worker the worker thread to run the casting and handling upon.
    * @arg priority the priority of the casting and handling task.
    * @return a reference to this object for daisy chains.
    */
    template<typename CastType>
    EventProcessor& bind_castable(KeyType key_value, std::function<void(CastType*)> functor, size_t worker=0, uint64_t priority=0)
    {
        event_handlers.emplace(
            key_value, 
            detail::GenericFunction{ [=](void* data){ functor(static_cast<CastType*>(data)); }, worker, priority}
        );
        return *this;
    }

    /**
    * Posts a new event with the given key_value to the RFUS.
    * @arg key_value the identifying value of the event handler.
    * @arg data a pointer to the data to be passed into the event handler.
    * @return a reference to this object for daisy chaining.
    */
    EventProcessor& post_event(KeyType key_value, void* data = nullptr)
    {
        auto find_itr = event_handlers.find(key_value);
        if(find_itr != event_handlers.end())
        {
            detail::GenericFunction& func = find_itr->second;
            rfus->post(Task([=] () { func.func(data); }, func.worker, func.priority));
        }
        return *this;
    }
   
private: 
    RFUSInterface* rfus;
    std::unordered_map<KeyType, detail::GenericFunction> event_handlers;
};

}
