#pragma once
#include <Iris/Platform/Vulkan/Buffer.hpp>
#include <Iris/Platform/Vulkan/UploadContext.hpp>

namespace Iris::Vulkan {
    class Texture final {
    public:
        Texture(const vk::Device& device, const vk::PhysicalDevice& physicalDevice,
                const std::shared_ptr<UploadContext>& ctx, std::string_view path, uint32_t desiredChannels = 3);

        Texture(Texture&& rhs) noexcept;

        [[nodiscard]] vk::DescriptorImageInfo GetDescriptor() const;
        ~Texture();
    private:
        glm::uvec2 m_Size{};
        vk::Sampler m_Sampler;
        vk::Device m_Device;
        std::unique_ptr<Buffer<float>> m_StagingBuffer;
        vk::Image m_Image;
        vk::DeviceMemory m_Memory;
        vk::ImageView m_ImageView;
    };
}