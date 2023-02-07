#include "UploadContext.hpp"

#include <utility>

namespace Iris::Vulkan {
    UploadContext::UploadContext(std::shared_ptr<Context> ctx)
            : m_Ctx(std::move(ctx)) {
        m_CommandPool = m_Ctx->GetDevice()
                .createCommandPool(vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                                             m_Ctx->GetTransferQueueFamilyIndex()));
        m_CommandBuffer = m_Ctx->GetDevice().allocateCommandBuffers(
                vk::CommandBufferAllocateInfo(m_CommandPool, vk::CommandBufferLevel::ePrimary, 1)).front();

        m_TransferFence = m_Ctx->GetDevice().createFence(vk::FenceCreateInfo());
        m_TransferSemaphore = m_Ctx->GetDevice().createSemaphore(vk::SemaphoreCreateInfo());
    }

    void UploadContext::SubmitCommand(const std::function<void(vk::CommandBuffer&)>& function) {
        m_CommandBuffer.begin(vk::CommandBufferBeginInfo(
                vk::CommandBufferUsageFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)));

        function(m_CommandBuffer);

        m_CommandBuffer.end();

        vk::SubmitInfo submitInfo({}, {}, m_CommandBuffer);

        m_Ctx->GetTransferQueue().submit(submitInfo, m_TransferFence);

        while (vk::Result::eTimeout == m_Ctx->GetDevice().waitForFences(m_TransferFence, VK_TRUE, 100000000));
        m_Ctx->GetDevice().resetFences(m_TransferFence);

        m_CommandBuffer.reset();
    }

    UploadContext::~UploadContext() {
        m_Ctx->GetDevice().destroyFence(m_TransferFence);
        m_Ctx->GetDevice().destroySemaphore(m_TransferSemaphore);
        m_Ctx->GetDevice().freeCommandBuffers(m_CommandPool, m_CommandBuffer);
        m_Ctx->GetDevice().destroyCommandPool(m_CommandPool);
    }
}