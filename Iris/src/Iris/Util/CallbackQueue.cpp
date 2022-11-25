#include "CallbackQueue.hpp"

namespace Iris {
    void CallbackQueue::PushFn(const std::function<void(void)>& fn) {
        std::scoped_lock lock(m_Mutex);
        m_Queue.emplace(fn);
    }

    void CallbackQueue::Flush() {
        std::scoped_lock lock(m_Mutex);
        while (!m_Queue.empty()) {
            auto item = m_Queue.front();
            item();
            m_Queue.pop();
        }
    }
}