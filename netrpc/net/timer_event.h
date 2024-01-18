#ifndef NETRPC_NET_TIMEREVENt
#define NETRPC_NET_TIMEREVENt

#include <functional>
#include <memory>

namespace netrpc {

class TimerEvent {
public:
    using TimerEventPtr = std::shared_ptr<TimerEvent>;

    TimerEvent(int interval, bool m_is_repeated, std::function<void()> cb);

    int64_t getArriveTime() const {
        return m_arrive_time;
    }

    void setCancled(bool value) {
        m_is_cancled = value;
    }

    bool isCancled() {
        return m_is_cancled;
    }

    bool isRepeated() {
        return m_is_repeated;
    }

    std::function<void()> getCallBack() {
        return m_task;
    }

    void resetArriveTime();

private:
    int64_t m_arrive_time;
    int64_t m_interval;
    bool m_is_repeated {false};
    bool m_is_cancled {false};

    std::function<void()> m_task;
};

}

#endif