#ifndef _TAREA2_EVENT_SOURCE_HPP
#define _TAREA2_EVENT_SOURCE_HPP

#include <list>
#include "EventListener.hpp"

namespace Tarea2
{

class EventSource
{
public:
    EventSource();
    ~EventSource();

    void registerListener(EventListener *listener);
    void unregisterListener(EventListener *listener);

    void fire(Event *event);

private:
    typedef std::list<EventListener*> Listeners;
    Listeners listeners;
};

}

#endif // _TAREA2_EVENT_SOURCE_HPP
