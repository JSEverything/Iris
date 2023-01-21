#include "Texture.hpp"
#include <stb_image.h>

namespace Iris::Vulkan {
    Texture::Texture(const vk::Device& device, const vk::PhysicalDevice& physicalDevice,
                     const std::shared_ptr<UploadContext>& ctx, std::string_view path, uint32_t desiredChannels)
            : m_Device(device) {

        int width, height, channels;
        float* data = stbi_loadf(path.data(), &width, &height, &channels, static_cast<int>(desiredChannels));
        if (data == nullptr) {
            Log::Core::Error("Failed to load texture {}, reason: ", path, stbi_failure_reason());
            std::exit(1);
        };

        m_Size = { width, height };

        m_StagingBuffer = std::make_unique<Buffer<float>>(device, physicalDevice,
                                                          vk::BufferUsageFlagBits::eTransferSrc,
                                                          data, desiredChannels * width * height);
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

        vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(imageFormat);

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
        m_Image = device.createImage(imageCreateInfo);

        vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();
        vk::MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(m_Image);
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
        m_Memory = device.allocateMemory(vk::MemoryAllocateInfo(memoryRequirements.size, typeIndex));

        device.bindImageMemory(m_Image, m_Memory, 0);

        m_ImageView = device.createImageView(vk::ImageViewCreateInfo(
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

        ctx->SubmitCommand([&](vk::CommandBuffer& buf) {
            vk::ImageMemoryBarrier imageMemoryBarrier(
                    vk::AccessFlags(), vk::AccessFlagBits::eTransferWrite, vk::ImageLayout::eUndefined,
                    vk::ImageLayout::eTransferDstOptimal, {}, {}, m_Image, imageSubresourceRange);

            buf.pipelineBarrier(vk::PipelineStageFlags(vk::PipelineStageFlagBits::eTransfer),
                                vk::PipelineStageFlags(vk::PipelineStageFlagBits::eTransfer), vk::DependencyFlags(),
                                {}, {}, imageMemoryBarrier);
        });

        ctx->SubmitCommand([&](vk::CommandBuffer& buf) {
            buf.copyBufferToImage(m_StagingBuffer->m_Buffer, m_Image, vk::ImageLayout::eTransferDstOptimal,
                                  copyRegion);

        });
        ctx->SubmitCommand([&](vk::CommandBuffer& buf) {
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

        m_Sampler = m_Device.createSampler(vk::SamplerCreateInfo(vk::SamplerCreateFlags(

        )));
    }

    Texture::Texture(Texture&& rhs) noexcept: m_Size(rhs.m_Size), m_Sampler(rhs.m_Sampler),
                                              m_Device(rhs.m_Device),
                                              m_StagingBuffer(std::move(rhs.m_StagingBuffer)),
                                              m_Image(rhs.m_Image), m_Memory(rhs.m_Memory),
                                              m_ImageView(rhs.m_ImageView) {
        rhs.m_Sampler = nullptr;
        rhs.m_Device = nullptr;
        rhs.m_Image = nullptr;
        rhs.m_Memory = nullptr;
        rhs.m_ImageView = nullptr;
    }

    vk::DescriptorImageInfo Texture::GetDescriptor() const {
        return { m_Sampler, m_ImageView, vk::ImageLayout::eShaderReadOnlyOptimal };
    }

    Texture::~Texture() {
        if (!m_Device) return;
        m_Device.destroySampler(m_Sampler);
        m_Device.destroyImageView(m_ImageView);
        m_Device.freeMemory(m_Memory);
        m_Device.destroyImage(m_Image);
    }
}