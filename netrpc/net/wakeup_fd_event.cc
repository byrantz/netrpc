#include <unistd.h>
#include "netrpc/net/wakeup_fd_event.h"
#include "netrpc/common/log.h"

namespace netrpc {
WakeUpFdEvent::WakeUpFdEvent(int fd) : FdEvent(fd) {

}

WakeUpFdEvent::~WakeUpFdEvent() {

}

void WakeUpFdEvent::wakeup() {
    char buf[8] = {'a'};
    int rt = write(m_fd, buf, 8);
    if (rt != 8) {
        ERRORLOG("write to wakeup fd less than 8 bytes, df[%d]", m_fd);
    }

    DEBUGLOG("sucess write 8 bytes");
}

}