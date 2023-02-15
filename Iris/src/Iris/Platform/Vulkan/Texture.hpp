#pragma once
#include "Iris/Platform/Vulkan/Context.hpp"
#include "Iris/Platform/Vulkan/Buffer.hpp"
#include "Iris/Platform/Vulkan/UploadContext.hpp"

namespace Iris::Vulkan {
    template <typename T>
    class Texture final {
    public:
        Texture(std::shared_ptr<Context> ctx, std::shared_ptr<UploadContext> uctx,
                std::string_view path, uint32_t desiredChannels = 3);

        Texture(std::shared_ptr<Context> ctx, std::shared_ptr<UploadContext> uctx,
                glm::uvec2 size, vk::Format format, vk::ImageUsageFlags flags) : m_Ctx(std::move(ctx)),
                                                                                 m_UCtx(std::move(uctx)),
                                                                                 m_Size(size) {
            vk::FormatProperties formatProperties = m_Ctx->GetPhysDevice().getFormatProperties(format);

            vk::ImageTiling tiling;
            if (formatProperties.linearTilingFeatures &
                (vk::FormatFeatureFlagBits::eTransferSrc | vk::FormatFeatureFlagBits::eColorAttachment)) {
                tiling = vk::ImageTiling::eLinear;
            } else if (formatProperties.optimalTilingFeatures &
                       (vk::FormatFeatureFlagBits::eTransferSrc | vk::FormatFeatureFlagBits::eColorAttachment)) {
                tiling = vk::ImageTiling::eOptimal;
            } else {
                Log::Core::Critical(
                        "ColorAttachment | TransferSrc  is not supported for texture format.");
                std::exit(1);
            }

            vk::ImageCreateInfo imageCreateInfo(vk::ImageCreateFlags(),
                                                vk::ImageType::e2D,
                                                format,
                                                vk::Extent3D(m_Size.x, m_Size.y, 1),
                                                1,
                                                1,
                                                vk::SampleCountFlagBits::e1,
                                                vk::ImageTiling::eOptimal,
                                                flags);
            m_Image = m_Ctx->GetDevice().createImage(imageCreateInfo);

            vk::PhysicalDeviceMemoryProperties memoryProperties = m_Ctx->GetPhysDevice().getMemoryProperties();
            vk::MemoryRequirements memoryRequirements = m_Ctx->GetDevice().getImageMemoryRequirements(m_Image);
            uint32_t typeBits = memoryRequirements.memoryTypeBits;
            auto typeIndex = uint32_t(~0);
            for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
                if ((typeBits & 1) &&
                    ((memoryProperties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal) ==
                     vk::MemoryPropertyFlagBits::eDeviceLocal)) {
                    typeIndex = i;
                    break;
                }
                typeBits >>= 1;
            }
            assert(typeIndex != uint32_t(~0));
            m_MemorySize = memoryRequirements.size;
            m_Memory = m_Ctx->GetDevice().allocateMemory(vk::MemoryAllocateInfo(m_MemorySize, typeIndex));

            m_Ctx->GetDevice().bindImageMemory(m_Image, m_Memory, 0);

            m_ImageView = m_Ctx->GetDevice().createImageView(vk::ImageViewCreateInfo(
                    vk::ImageViewCreateFlags(), m_Image, vk::ImageViewType::e2D, format, {},
                    { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }));

            m_Sampler = m_Ctx->GetDevice().createSampler(vk::SamplerCreateInfo(vk::SamplerCreateFlags(

            )));
        }

        Texture(Texture&& rhs) noexcept: m_Size(rhs.m_Size), m_Sampler(rhs.m_Sampler),
                                         m_Ctx(rhs.m_Ctx),
                                         m_UCtx(rhs.m_UCtx),
                                         m_StagingBuffer(std::move(rhs.m_StagingBuffer)),
                                         m_Image(rhs.m_Image), m_Memory(rhs.m_Memory),
                                         m_ImageView(rhs.m_ImageView) {
            rhs.m_Sampler = nullptr;
            rhs.m_Image = nullptr;
            rhs.m_Memory = nullptr;
            rhs.m_ImageView = nullptr;
        }

        void CopyToStagingBuffer() {
            m_StagingBuffer.reset();
            m_StagingBuffer = std::make_unique<Buffer<T>>(
                    m_Ctx, vk::BufferUsageFlagBits::eTransferDst, m_MemorySize);

            vk::BufferImageCopy copyRegion(
                    0,
                    m_Size.x,
                    m_Size.y,
                    vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
                    vk::Offset3D(0, 0, 0),
                    vk::Extent3D(m_Size.x, m_Size.y, 1));

            vk::ImageSubresourceRange imageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

            m_UCtx->SubmitCommand([&](vk::CommandBuffer& buf) {
                vk::ImageMemoryBarrier imageMemoryBarrier(
                        vk::AccessFlags(), vk::AccessFlagBits::eTransferRead, vk::ImageLayout::eColorAttachmentOptimal,
                        vk::ImageLayout::eTransferSrcOptimal, {}, {}, m_Image, imageSubresourceRange);

                buf.pipelineBarrier(vk::PipelineStageFlags(vk::PipelineStageFlagBits::eTransfer),
                                    vk::PipelineStageFlags(vk::PipelineStageFlagBits::eTransfer), vk::DependencyFlags(),
                                    {}, {}, imageMemoryBarrier);
            });

            m_UCtx->SubmitCommand([&](vk::CommandBuffer& buf) {
                buf.copyImageToBuffer(m_Image, vk::ImageLayout::eTransferSrcOptimal, m_StagingBuffer->m_Buffer,
                                      copyRegion);
            });
            m_UCtx->SubmitCommand([&](vk::CommandBuffer& buf) {
                vk::ImageMemoryBarrier imageMemoryBarrier(
                        vk::AccessFlagBits::eTransferRead, vk::AccessFlags(),
                        vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eColorAttachmentOptimal,
                        {}, {}, m_Image, imageSubresourceRange);

                buf.pipelineBarrier(vk::PipelineStageFlags(vk::PipelineStageFlagBits::eTransfer),
                                    vk::PipelineStageFlags(vk::PipelineStageFlagBits::eTransfer),
                                    vk::DependencyFlags(),
                                    {},
                                    {},
                                    imageMemoryBarrier);
            });
        }

        T* MapStagingBuffer() {
            return m_StagingBuffer->Map();
        }

        void UnmapStagingBuffer() {
            m_StagingBuffer->Unmap();
        }

        [[nodiscard]] vk::DescriptorImageInfo GetDescriptor() const {
            return { m_Sampler, m_ImageView, vk::ImageLayout::eShaderReadOnlyOptimal };
        }

        ~Texture() {
            if (!m_Ctx) return;
            m_Ctx->GetDevice().destroySampler(m_Sampler);
            m_Ctx->GetDevice().destroyImageView(m_ImageView);
            m_Ctx->GetDevice().freeMemory(m_Memory);
            m_Ctx->GetDevice().destroyImage(m_Image);
        }

    private:
        std::shared_ptr<Context> m_Ctx{};
        std::shared_ptr<UploadContext> m_UCtx{};
        glm::uvec2 m_Size{};
        size_t m_MemorySize{};
        vk::Sampler m_Sampler;
        std::unique_ptr<Buffer<T>> m_StagingBuffer;
        vk::Image m_Image;
        vk::DeviceMemory m_Memory;
        vk::ImageView m_ImageView;
    };
}