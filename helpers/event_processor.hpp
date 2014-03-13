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

template<typename KeyType>
struct EventProcessor
{
public:

    EventProcessor(RFUSInterface* rfus)
        : rfus(rfus)
    {
    }

    template<typename CastType, typename EventDataType>
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
    }

    template<typename CastType>
    EventProcessor& bind_castable(KeyType key_value, std::function<void(CastType*)> functor, size_t worker=0, uint64_t priority=0)
    {
        event_handlers.emplace(
            key_value, 
            detail::GenericFunction{ [=](void* data){ functor(static_cast<CastType*>(data)); }, worker, priority}
        );
    }

    void unbind_event(KeyType key_value)
    {
        auto find_itr = event_handlers.find(key_value);
        if(find_itr != event_handlers.end())
        {
            event_handlers.erase(find_itr);
        }
    }

    void post_event(KeyType key_value, void* data)
    {
        auto find_itr = event_handlers.find(key_value);
        if(find_itr != event_handlers.end())
        {
            detail::GenericFunction& func = find_itr->second;
            rfus->post(Task([=] () { func.func(data); }, func.worker, func.priority));
        }
    }
   
private: 
    RFUSInterface* rfus;
    std::unordered_map<KeyType, detail::GenericFunction> event_handlers;
};

}
