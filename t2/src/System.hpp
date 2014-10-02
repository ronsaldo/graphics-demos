#ifndef TAREA2_SYSTEM_HPP
#define TAREA2_SYSTEM_HPP

#include <string>
#include "EventListener.hpp"
#include "EventSource.hpp"

namespace Tarea2
{

/**
 * Tarea2 system specific data.
 */
class System: public EventSource
{
public:
    System();
    ~System();

    int getWidth() const;
    int getHeight() const;

    void setTitle(const std::string &title);

    bool initialize(int argc, const char *argv[]);
    void shutdown();

    void mainloop();
    void quit();
    
private:
    void sendEvents();

    bool quitting_;
    int width_, height_;
};

} // namespace Rot

#endif //TAREA2_SYSTEM_HPP
