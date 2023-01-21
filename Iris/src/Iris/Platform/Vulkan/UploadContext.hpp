#pragma once
#include <vulkan/vulkan.hpp>

namespace Iris::Vulkan {
    class UploadContext final {
    public:
        UploadContext(const vk::Device& device, const vk::Queue& queue, uint32_t queueFamilyIndex);

        void SubmitCommand(const std::function<void(vk::CommandBuffer&)>& function);

        ~UploadContext();
    private:
        vk::Device m_Device;
        vk::CommandPool m_CommandPool;
        vk::CommandBuffer m_CommandBuffer;
        vk::Queue m_TransferQueue;

        vk::Fence m_TransferFence;
        vk::Semaphore m_TransferSemaphore;
    };
}