#ifndef NETRPC_NET_TIMER_H
#define NETRPC_NET_TIMER_H

#include <map>
#include <mutex>
#include "netrpc/net/fd_event.h"
#include "netrpc/net/timer_event.h"

namespace netrpc {

class Timer : public FdEvent {
public:
    Timer();

    ~Timer();

    void addTimerEvent(TimerEvent::TimerEventPtr event);

    void deleteTimerEvent(TimerEvent::TimerEventPtr event);

    void onTimer(); // 当发送了 IO 事件后，eventloop 会执行这个回调函数

private:
    void resetArriveTime();

private:
    // 保存对应的时间和定时任务
    std::multimap<int64_t, TimerEvent::TimerEventPtr> m_pending_events;
    std::mutex m_mutex;
};

}

#endif