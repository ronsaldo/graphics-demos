#ifndef TAREA2_EVENT_LISTENER_HPP
#define TAREA2_EVENT_LISTENER_HPP

#include "Interface.hpp"
#include "Event.hpp"

namespace Tarea2
{

/**
 * Event listener.
 */
struct EventListener: Interface
{
    virtual void handleEvent(Event *event) = 0;
};

}

#endif // TAREA2_EVENT_LISTENER_HPP
