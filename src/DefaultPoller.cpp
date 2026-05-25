#include "Poller.h"
#include "EPollPoller.h"
#include <stdlib.h>

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
    if(::getenv("MODUO_USE_POLL"))
    {
        return nullptr;
    }
    else
    {
        return new EPollPoller(loop);
    }
}