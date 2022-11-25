#pragma once
#include <queue>
#include <functional>

namespace Iris {
    class CallbackQueue {
    public:
        void PushFn(const std::function<void(void)>& fn);
        void Flush();
    private:
        std::queue<std::function<void(void)>> m_Queue;
        std::mutex m_Mutex;
    };
}
