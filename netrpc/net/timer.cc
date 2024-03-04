#include <sys/timerfd.h>
#include <string.h>
#include "netrpc/net/timer.h"
#include "netrpc/common/log.h"
#include "netrpc/common/util.h"

namespace netrpc {

Timer::Timer() : FdEvent() {
    m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    DEBUGLOG("timer fd = %d", m_fd);

    // 把 fd 可读事件放到了 eventloop 上监听
    // 这里是设置监听事件，以及对应的回调函数
    listen(FdEvent::IN_EVENT, std::bind(&Timer::onTimer, this));
}

Timer::~Timer() {

}

void Timer::onTimer() {
    // 处理缓冲区数据，防止下一次继续触发可读事件
    DEBUGLOG("ontimer");
    char buf[8];
    while (1) {
        if ((read(m_fd, buf, 8) == -1) && errno == EAGAIN) {
            break;
        }
    }

    // 执行定时任务
    int64_t now = getNowMs();

    std::vector<TimerEvent::TimerEventPtr> tmps;
    std::vector<std::pair<int64_t, std::function<void()>>> tasks;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_pending_events.begin();

        for (it = m_pending_events.begin(); it != m_pending_events.end(); ++it) {
            if ((*it).first <= now) {
                if (!(*it).second->isCancled()) {
                    tmps.push_back((*it).second);
                    tasks.push_back(std::make_pair((*it).second->getArriveTime(), (*it).second->getCallBack()));
                }
            } else {
                break;
            }
        }

        m_pending_events.erase(m_pending_events.begin(), it);
    }

    // 需要把重复的 Event 再次添加进去
    for (auto i = tmps.begin(); i != tmps.end(); ++i ) {
        if ((*i)->isRepeated()) {
            // 调整 arriveTime
            (*i)->resetArriveTime();
            addTimerEvent(*i);
        }
    }

    resetArriveTime();

    for (auto i : tasks) {
        if (i.second) {
            i.second();
        }
    }
}

void Timer::resetArriveTime() {
    std::unique_lock<std::mutex> lock(m_mutex);
    auto tmp = m_pending_events;
    lock.unlock();


    if (tmp.size() == 0) {
        return;
    }

    int64_t now = getNowMs();

    auto it = tmp.begin();
    int64_t interval = 0;
    if (it->second->getArriveTime() > now) {
        interval = it->second->getArriveTime() - now;
    } else {
        interval = 100;
    }

    timespec ts;
    memset(&ts, 0, sizeof(ts));
    ts.tv_sec = interval / 1000;
    ts.tv_nsec = (interval % 1000) * 1000000;

    itimerspec value;
    memset(&value, 0, sizeof(value));
    value.it_value = ts;

    // timerfd_settime 设置超时定时器
    // 当定时器超时时，它的文件描述符会变为可读状态，可以通过 epoll 或类似的I/O多路复用机制来检测这一状态。
    // 在 Timer 类的 onTimer 方法中，这一点被用来触发和处理定时事件。
    int rt = timerfd_settime(m_fd, 0, &value, NULL);
    if (rt != 0) {
        ERRORLOG("timerfd_settime error, errno=%d, error=%s", errno, strerror(errno));
    }
    DEBUGLOG("timer reset to %lld", now + interval);
}

void Timer::addTimerEvent(TimerEvent::TimerEventPtr event) {
    bool is_reset_timerfd = false;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_pending_events.empty()) {
            is_reset_timerfd = true;
        } else {
            auto it = m_pending_events.begin();
            if ((*it).second->getArriveTime() > event->getArriveTime()) {
                is_reset_timerfd = true;
            }
        }
        m_pending_events.emplace(event->getArriveTime(), event);
    }

    if (is_reset_timerfd) {
        resetArriveTime();
    }
}

void Timer::deleteTimerEvent(TimerEvent::TimerEventPtr event) {
    event->setCancled(true);

    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto begin = m_pending_events.lower_bound(event->getArriveTime());
        auto end = m_pending_events.upper_bound(event->getArriveTime());

        auto it = begin;
        for (it = begin; it != end; ++it) {
            if (it->second == event) {
                break;
            }
        }

        if (it != end) {
            m_pending_events.erase(it);
        }
    }
}

}