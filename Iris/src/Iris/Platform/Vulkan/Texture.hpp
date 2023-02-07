#pragma once
#include "Iris/Platform/Vulkan/Context.hpp"
#include "Iris/Platform/Vulkan/Buffer.hpp"
#include "Iris/Platform/Vulkan/UploadContext.hpp"

namespace Iris::Vulkan {
    class Texture final {
    public:
        Texture(std::shared_ptr<Context>  ctx, const std::shared_ptr<UploadContext>& uctx,
                std::string_view path, uint32_t desiredChannels = 3);

        Texture(Texture&& rhs) noexcept;

        [[nodiscard]] vk::DescriptorImageInfo GetDescriptor() const;
        ~Texture();
    private:
        std::shared_ptr<Context> m_Ctx{};
        glm::uvec2 m_Size{};
        vk::Sampler m_Sampler;
        std::unique_ptr<Buffer<float>> m_StagingBuffer;
        vk::Image m_Image;
        vk::DeviceMemory m_Memory;
        vk::ImageView m_ImageView;
    };
}