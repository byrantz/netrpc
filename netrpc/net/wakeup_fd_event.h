#ifndef NETRPC_NET_WAKEUP_FDEVENT_H
#define NETRPC_NET_WAKEUP_FDEVENT_H

#include "netrpc/net/fd_event.h"

namespace netrpc {

class WakeUpFdEvent : public FdEvent {
public:
    WakeUpFdEvent(int fd);

    ~WakeUpFdEvent();

    void wakeup();
private:

};

}

#endif