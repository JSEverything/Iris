#include "Texture.hpp"
#include <stb_image.h>

namespace Iris::Vulkan {
    template <>
    Texture<float>::Texture(std::shared_ptr<Context> ctx, std::shared_ptr<UploadContext> uctx,
                            std::string_view path, uint32_t desiredChannels)
            : m_Ctx(std::move(ctx)), m_UCtx(std::move(uctx)) {

        int width, height, channels;
        float* data = stbi_loadf(path.data(), &width, &height, &channels, static_cast<int>(desiredChannels));
        if (data == nullptr) {
            Log::Core::Error("Failed to load texture {}, reason: ", path, stbi_failure_reason());
            std::exit(1);
        };

        m_Size = { width, height };

        m_StagingBuffer = std::make_unique<Buffer<float>>(
                m_Ctx, vk::BufferUsageFlagBits::eTransferSrc, data, desiredChannels * width * height);
        stbi_image_free(data);

        vk::Format imageFormat;
        switch (desiredChannels) {
            case 4:
                imageFormat = vk::Format::eR32G32B32A32Sfloat;
                break;
            case 3:
                imageFormat = vk::Format::eR32G32B32Sfloat;
                break;
            case 2:
                imageFormat = vk::Format::eR32G32Sfloat;
                break;
            case 1:
                imageFormat = vk::Format::eR32Sfloat;
                break;
            default:
                Log::Core::Error("Unknown format for texture {}", path);
                std::exit(-1);
        }

        vk::FormatProperties formatProperties = m_Ctx->GetPhysDevice().getFormatProperties(imageFormat);

        vk::ImageTiling tiling;
        if (formatProperties.linearTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage) {
            tiling = vk::ImageTiling::eLinear;
        } else if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage) {
            tiling = vk::ImageTiling::eOptimal;
        } else {
            Log::Core::Critical("SampledImage is not supported for texture format.");
            std::exit(1);
        }

        vk::ImageCreateInfo imageCreateInfo(vk::ImageCreateFlags(),
                                            vk::ImageType::e2D,
                                            imageFormat,
                                            vk::Extent3D(m_Size.x, m_Size.y, 1),
                                            1,
                                            1,
                                            vk::SampleCountFlagBits::e1,
                                            tiling,
                                            vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);
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
                vk::ImageViewCreateFlags(), m_Image, vk::ImageViewType::e2D, imageFormat, {},
                { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }));

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
                    vk::AccessFlags(), vk::AccessFlagBits::eTransferWrite, vk::ImageLayout::eUndefined,
                    vk::ImageLayout::eTransferDstOptimal, {}, {}, m_Image, imageSubresourceRange);

            buf.pipelineBarrier(vk::PipelineStageFlags(vk::PipelineStageFlagBits::eTransfer),
                                vk::PipelineStageFlags(vk::PipelineStageFlagBits::eTransfer), vk::DependencyFlags(),
                                {}, {}, imageMemoryBarrier);
        });

        m_UCtx->SubmitCommand([&](vk::CommandBuffer& buf) {
            buf.copyBufferToImage(m_StagingBuffer->m_Buffer, m_Image, vk::ImageLayout::eTransferDstOptimal,
                                  copyRegion);

        });
        m_UCtx->SubmitCommand([&](vk::CommandBuffer& buf) {
            vk::ImageMemoryBarrier imageMemoryBarrier(
                    vk::AccessFlagBits::eTransferWrite, vk::AccessFlags(),
                    vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
                    {}, {}, m_Image, imageSubresourceRange);

            buf.pipelineBarrier(vk::PipelineStageFlags(vk::PipelineStageFlagBits::eTransfer),
                                vk::PipelineStageFlags(vk::PipelineStageFlagBits::eTransfer),
                                vk::DependencyFlags(),
                                {},
                                {},
                                imageMemoryBarrier);
        });

        m_StagingBuffer.reset();

        m_Sampler = m_Ctx->GetDevice().createSampler(vk::SamplerCreateInfo(vk::SamplerCreateFlags(

        )));
    }
}