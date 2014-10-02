#ifndef TAREA2_EVENT_HPP
#define TAREA2_EVENT_HPP

namespace Tarea2
{
    /**
     * Event base class
     */
    class Event
    {
    public:
        enum Type
        {
            FrameUpdate = 0,
            FrameDraw,
            KeyDown,
            KeyUp,
            MouseMove,
            MouseEnter,
            MouseLeave
        };

        Event(Type type)
            : type(type) {}
        ~Event() {}

        Type getType() const
        {
            return type;
        }

    private:
        Type type;
    };

    /**
     * Key board event
     */
    class KeyEvent: public Event
    {
    public:
        KeyEvent(Type type, int keyCode)
            : Event(type), keyCode(keyCode) {}
        ~KeyEvent() {}
        
        int getKeyCode() const
        {
            return keyCode;
        }

    private:
        int keyCode;
    };

    /**
     * Mouse move event.
     */
    class MouseMoveEvent: public Event
    {
    public:
        MouseMoveEvent(Type type, int x, int y)
            : Event(type), x(x), y(y) {}
        ~MouseMoveEvent() {}

        int getX() const
        {
            return x;
        }

        int getY() const
        {
            return y;
        }

    private:
        int x, y;
    };

    /**
     * Frame update event.
     */
    class FrameUpdateEvent: public Event
    {
    public:
        FrameUpdateEvent(Type type, float delta)
            : Event(type), delta(delta) {}
        ~FrameUpdateEvent() {}

        float getDelta() const
        {
            return delta;
        }

    private:
        float delta;
    };
}

#endif //TAREA2_EVENT_HPP

