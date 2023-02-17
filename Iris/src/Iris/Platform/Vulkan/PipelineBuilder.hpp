#pragma once
#include <vulkan/vulkan.hpp>

namespace Iris::Vulkan {
    class PipelineBuilder final {
    public:
        class Pipeline;

        explicit PipelineBuilder(const vk::Device& mDevice);
        PipelineBuilder& SetVertexInputAttributes(const vk::VertexInputBindingDescription&& description,
                                                  const std::vector<vk::VertexInputAttributeDescription>&& attributes);
        PipelineBuilder& AddVertexShader(std::string path);
        PipelineBuilder& AddFragmentShader(std::string path);
        PipelineBuilder& AddUniform(uint32_t set, uint32_t binding, vk::ShaderStageFlags stage, uint32_t count = 1);
        PipelineBuilder&
        AddStorageBuffer(uint32_t set, uint32_t binding, vk::ShaderStageFlags stage, uint32_t count = 1);
        PipelineBuilder& AddImage(uint32_t set, uint32_t binding, vk::ShaderStageFlags stage, uint32_t count = 1);
        PipelineBuilder& AddPushConstant(vk::ShaderStageFlags stage, size_t size);
        std::unique_ptr<Pipeline> Build(vk::RenderPass& renderPass);

        PipelineBuilder& Clear();
        ~PipelineBuilder();
    private:
        std::optional<vk::ShaderModule> loadShaderModule(std::string path);
    private:
        vk::Device m_Device;
        vk::DescriptorPool m_DescriptorPool;
        size_t m_PushConstantOffset = 0u;

        vk::VertexInputBindingDescription m_VertexInputBindingDescription;
        std::vector<vk::VertexInputAttributeDescription> m_AttributeDescriptions;

        std::map<uint32_t, std::map<uint32_t, vk::DescriptorSetLayoutBinding>> m_DescriptorSetLayoutBindings;

        std::vector<vk::ShaderModule> m_VertexShaders;
        std::vector<vk::ShaderModule> m_FragmentShaders;
        std::vector<vk::PushConstantRange> m_PushConstants;
    public:
        class Pipeline final {
        public:
            vk::PipelineLayout pipelineLayout;
            vk::Pipeline pipeline;
            std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
            std::vector<vk::DescriptorSet> descriptorSets;

            void UpdateBuffer(uint32_t set, uint32_t binding, vk::DescriptorBufferInfo info, uint32_t index = 0);
            void UpdateImage(uint32_t set, uint32_t binding, vk::DescriptorImageInfo info, uint32_t index = 0);

            ~Pipeline();
        private:
            Pipeline(const vk::Device& device, const vk::DescriptorPool& descriptorPool,
                     const std::map<uint32_t, std::map<uint32_t, vk::DescriptorSetLayoutBinding>>& descriptorSetLayoutBindings);

            vk::Device device;
            vk::DescriptorPool descriptorPool;
            const std::map<uint32_t, std::map<uint32_t, vk::DescriptorSetLayoutBinding>> descriptorSetLayoutBindings; // This has to be a copy

            friend PipelineBuilder;
        };
    };
}
