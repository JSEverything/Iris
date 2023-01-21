#include "UploadContext.hpp"

namespace Iris::Vulkan {
    UploadContext::UploadContext(const vk::Device& device, const vk::Queue& queue, uint32_t queueFamilyIndex)
            : m_Device(device), m_TransferQueue(queue) {
        m_CommandPool = m_Device
                .createCommandPool(vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                                             queueFamilyIndex));
        m_CommandBuffer = m_Device.allocateCommandBuffers(
                vk::CommandBufferAllocateInfo(m_CommandPool, vk::CommandBufferLevel::ePrimary, 1)).front();

        m_TransferFence = m_Device.createFence(vk::FenceCreateInfo());
        m_TransferSemaphore = m_Device.createSemaphore(vk::SemaphoreCreateInfo());
    }

    void UploadContext::SubmitCommand(const std::function<void(vk::CommandBuffer&)>& function) {
        m_CommandBuffer.begin(vk::CommandBufferBeginInfo(
                vk::CommandBufferUsageFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)));

        function(m_CommandBuffer);

        m_CommandBuffer.end();
        
        vk::SubmitInfo submitInfo({}, {}, m_CommandBuffer);

        m_TransferQueue.submit(submitInfo, m_TransferFence);

        while (vk::Result::eTimeout == m_Device.waitForFences(m_TransferFence, VK_TRUE, 100000000));
        m_Device.resetFences(m_TransferFence);

        m_CommandBuffer.reset();
    }

    UploadContext::~UploadContext() {
        m_Device.destroyFence(m_TransferFence);
        m_Device.destroySemaphore(m_TransferSemaphore);
        m_Device.freeCommandBuffers(m_CommandPool, m_CommandBuffer);
        m_Device.destroyCommandPool(m_CommandPool);
    }
}