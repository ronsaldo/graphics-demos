#ifndef T3_THREADING_HPP
#define T3_THREADING_HPP

#include <SDL/SDL_thread.h>

namespace T3
{

/**
 * SDL Mutex RAII wrapper.
 */
class Mutex
{
public:
    Mutex()
    {
        mutex = SDL_CreateMutex();
    }

    ~Mutex()
    {
        SDL_DestroyMutex(mutex);
    }

    void lock() const
    {
        SDL_LockMutex(mutex);
    }

    void unlock() const
    {
        SDL_UnlockMutex(mutex);
    }

    SDL_mutex *getHandle() const
    {
        return mutex;
    }

private:
    mutable SDL_mutex *mutex;
};

/**
 * SDL Mutex lock RAII wrapper.
 */
class Lock
{
public:
    Lock(const Mutex &mutex)
        : mutex(mutex)
    {
        mutex.lock();
    }

    ~Lock()
    {
        mutex.unlock();
    }

    const Mutex &getMutex()
    {
        return mutex;
    }

private:
    const Mutex &mutex;
};


/**
 * SDL condition RAII wrapper.
 */
class Condition
{
public:
    Condition()
    {
        cond = SDL_CreateCond();
    }

    ~Condition()
    {
        SDL_DestroyCond(cond);
    }

    void wait(Lock &lock)
    {
        SDL_CondWait(cond, lock.getMutex().getHandle());
    }

    void signal()
    {
        SDL_CondSignal(cond);
    }

    void broadcast()
    {
        SDL_CondBroadcast(cond);
    }

private:
    SDL_cond *cond;
};
}

#endif //T3_THREADING_HPP
