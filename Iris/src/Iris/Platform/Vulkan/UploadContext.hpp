#pragma once
#include <vulkan/vulkan.hpp>
#include "Iris/Platform/Vulkan/Context.hpp"

namespace Iris::Vulkan {
    class UploadContext final {
    public:
        UploadContext(std::shared_ptr<Context> ctx);

        void SubmitCommand(const std::function<void(vk::CommandBuffer&)>& function);

        ~UploadContext();
    private:
        std::shared_ptr<Context> m_Ctx;
        vk::CommandPool m_CommandPool;
        vk::CommandBuffer m_CommandBuffer;

        vk::Fence m_TransferFence;
        vk::Semaphore m_TransferSemaphore;
    };
}