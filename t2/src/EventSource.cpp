#include "EventSource.hpp"

namespace Tarea2
{

    EventSource::EventSource()
    {
    }

    EventSource::~EventSource()
    {
    }

    void EventSource::registerListener(EventListener *listener)
    {
        listeners.push_back(listener);
    }

    void EventSource::unregisterListener(EventListener *listener)
    {
        Listeners::iterator it = listeners.begin();
        while(it != listeners.end())
        {
            if(*it == listener)
            {
                listeners.erase(it++);
            }
            else
            {
                ++it;
            }
        }
    }

    void EventSource::fire(Event *event)
    {
        Listeners::iterator it = listeners.begin();
        for(; it != listeners.end(); ++it)
            (*it)->handleEvent(event);
    }
}

