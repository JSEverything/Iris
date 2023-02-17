#include "PipelineBuilder.hpp"

namespace Iris::Vulkan {
    PipelineBuilder::PipelineBuilder(const vk::Device& mDevice) : m_Device(mDevice) {
        std::vector<vk::DescriptorPoolSize> poolSizes = {
                { vk::DescriptorType::eSampler,              65536 },
                { vk::DescriptorType::eCombinedImageSampler, 65536 },
                { vk::DescriptorType::eSampledImage,         65536 },
                { vk::DescriptorType::eStorageImage,         65536 },
                { vk::DescriptorType::eUniformTexelBuffer,   65536 },
                { vk::DescriptorType::eStorageTexelBuffer,   65536 },
                { vk::DescriptorType::eUniformBuffer,        65536 },
                { vk::DescriptorType::eStorageBuffer,        65536 },
                { vk::DescriptorType::eUniformBufferDynamic, 65536 },
                { vk::DescriptorType::eStorageBufferDynamic, 65536 },
                { vk::DescriptorType::eInputAttachment,      65536 },
        };

        m_DescriptorPool = m_Device.createDescriptorPool(
                vk::DescriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet |
                                             vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind, 128, poolSizes));
    }

    PipelineBuilder& PipelineBuilder::SetVertexInputAttributes(const vk::VertexInputBindingDescription&& description,
                                                               const std::vector<vk::VertexInputAttributeDescription>&& attributes) {
        m_AttributeDescriptions = attributes;
        m_VertexInputBindingDescription = description;

        return *this;
    }

    PipelineBuilder& PipelineBuilder::AddVertexShader(std::string path) {
        auto&& shader = loadShaderModule(std::move(path));
        if (shader) m_VertexShaders.emplace_back(*shader);
        return *this;
    }

    PipelineBuilder& PipelineBuilder::AddFragmentShader(std::string path) {
        auto&& shader = loadShaderModule(std::move(path));
        if (shader) m_FragmentShaders.emplace_back(*shader);
        return *this;
    }

    PipelineBuilder&
    PipelineBuilder::AddUniform(uint32_t set, uint32_t binding, vk::ShaderStageFlags stage, uint32_t count) {
        m_DescriptorSetLayoutBindings[set][binding] = vk::DescriptorSetLayoutBinding(
                binding, vk::DescriptorType::eUniformBuffer, count, stage);

        return *this;
    }

    PipelineBuilder&
    PipelineBuilder::AddStorageBuffer(uint32_t set, uint32_t binding, vk::ShaderStageFlags stage, uint32_t count) {
        m_DescriptorSetLayoutBindings[set][binding] = vk::DescriptorSetLayoutBinding(
                binding, vk::DescriptorType::eStorageBuffer, count, stage);

        return *this;
    }

    PipelineBuilder&
    PipelineBuilder::AddImage(uint32_t set, uint32_t binding, vk::ShaderStageFlags stage, uint32_t count) {
        m_DescriptorSetLayoutBindings[set][binding] = vk::DescriptorSetLayoutBinding(
                binding, vk::DescriptorType::eCombinedImageSampler, count, stage);

        return *this;
    }

    PipelineBuilder& PipelineBuilder::AddPushConstant(vk::ShaderStageFlags stage, size_t size) {
        m_PushConstants.emplace_back(stage, m_PushConstantOffset, size);
        m_PushConstantOffset += size;
        return *this;
    }

    std::unique_ptr<PipelineBuilder::Pipeline> PipelineBuilder::Build(vk::RenderPass& renderPass) {
        auto out = std::unique_ptr<Pipeline>(new Pipeline(m_Device, m_DescriptorPool, m_DescriptorSetLayoutBindings));

        for (auto [set, bindings]: m_DescriptorSetLayoutBindings) {
            std::vector<vk::DescriptorSetLayoutBinding> temp;
            vk::DescriptorSetLayoutBindingFlagsCreateInfo setLayoutBindingsFlags = {};
            std::vector<vk::DescriptorBindingFlags> bindingFlags{};

            for (auto [binding, layout]: bindings) {
                if (layout.descriptorType == vk::DescriptorType::eUniformBuffer) {
                    bindingFlags.emplace_back(vk::DescriptorBindingFlagBits::ePartiallyBound);
                } else {
                    bindingFlags.emplace_back(vk::DescriptorBindingFlagBits::ePartiallyBound |
                                              vk::DescriptorBindingFlagBits::eUpdateAfterBind);
                }
                temp.emplace_back(layout);
            }
            setLayoutBindingsFlags.setBindingFlags(bindingFlags);

            out->descriptorSetLayouts.emplace_back(m_Device.createDescriptorSetLayout(
                    vk::DescriptorSetLayoutCreateInfo(vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
                                                      temp).setPNext(&setLayoutBindingsFlags)));
        }

        out->pipelineLayout = m_Device.createPipelineLayout(vk::PipelineLayoutCreateInfo(
                vk::PipelineLayoutCreateFlags(), out->descriptorSetLayouts, m_PushConstants));

        {
            std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos;
            for (auto& shader: m_VertexShaders) {
                pipelineShaderStageCreateInfos.emplace_back(
                        vk::PipelineShaderStageCreateFlags(),
                        vk::ShaderStageFlagBits::eVertex, shader, "main");
            }
            for (auto& shader: m_FragmentShaders) {
                pipelineShaderStageCreateInfos.emplace_back(
                        vk::PipelineShaderStageCreateFlags(),
                        vk::ShaderStageFlagBits::eFragment, shader, "main");
            }

            auto pipelineVertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo(
                    vk::PipelineVertexInputStateCreateFlags(),
                    m_VertexInputBindingDescription, m_AttributeDescriptions
            );

            vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
                    vk::PipelineInputAssemblyStateCreateFlags(),
                    vk::PrimitiveTopology::eTriangleList);

            vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(
                    vk::PipelineViewportStateCreateFlags(), 1,
                    nullptr, 1, nullptr);

            vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
                    vk::PipelineRasterizationStateCreateFlags(),  // flags
                    false,                                        // depthClampEnable
                    false,                                        // rasterizerDiscardEnable
                    vk::PolygonMode::eFill,                       // polygonMode
                    vk::CullModeFlagBits::eNone,                  // cullMode
                    vk::FrontFace::eClockwise,                    // frontFace
                    false,                                        // depthBiasEnable
                    0.0f,                                         // depthBiasConstantFactor
                    0.0f,                                         // depthBiasClamp
                    0.0f,                                         // depthBiasSlopeFactor
                    1.0f                                          // lineWidth
            );

            vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
                    vk::PipelineMultisampleStateCreateFlags(),  // flags
                    vk::SampleCountFlagBits::e1                 // rasterizationSamples
            );

            vk::StencilOpState stencilOpState(
                    vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways);
            vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
                    vk::PipelineDepthStencilStateCreateFlags(),  // flags
                    true,                                        // depthTestEnable
                    true,                                        // depthWriteEnable
                    vk::CompareOp::eLessOrEqual,                 // depthCompareOp
                    false,                                       // depthBoundTestEnable
                    false,                                       // stencilTestEnable
                    stencilOpState,                              // front
                    stencilOpState                               // back
            );

            vk::ColorComponentFlags colorComponentFlags(
                    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
                    vk::ColorComponentFlagBits::eA);

            vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentStateEnabled(
                    true,                               // blendEnable
                    vk::BlendFactor::eSrcAlpha,         // srcColorBlendFactor
                    vk::BlendFactor::eOneMinusSrcAlpha,              // dstColorBlendFactor
                    vk::BlendOp::eAdd,                  // colorBlendOp
                    vk::BlendFactor::eSrcAlpha,             // srcAlphaBlendFactor
                    vk::BlendFactor::eOneMinusSrcAlpha, // dstAlphaBlendFactor
                    vk::BlendOp::eAdd,                  // alphaBlendOp
                    colorComponentFlags                 // colorWriteMask
            );

            vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentStateDisabled(
                    false,                   // blendEnable
                    vk::BlendFactor::eZero,  // srcColorBlendFactor
                    vk::BlendFactor::eZero,  // dstColorBlendFactor
                    vk::BlendOp::eAdd,       // colorBlendOp
                    vk::BlendFactor::eZero,  // srcAlphaBlendFactor
                    vk::BlendFactor::eZero,  // dstAlphaBlendFactor
                    vk::BlendOp::eAdd,       // alphaBlendOp
                    colorComponentFlags      // colorWriteMask
            );

            std::array<vk::PipelineColorBlendAttachmentState, 2> pipelineColorBlendAttachments = {
                    pipelineColorBlendAttachmentStateEnabled, pipelineColorBlendAttachmentStateDisabled };

            vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
                    vk::PipelineColorBlendStateCreateFlags(),  // flags
                    false,                                     // logicOpEnable
                    vk::LogicOp::eNoOp,                        // logicOp
                    pipelineColorBlendAttachments,             // attachments
                    { { 1.0f, 1.0f, 1.0f, 1.0f } }             // blendConstants
            );

            std::array<vk::DynamicState, 2> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
            vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
                    vk::PipelineDynamicStateCreateFlags(),
                    dynamicStates);

            vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo(
                    vk::PipelineCreateFlags(),              // flags
                    pipelineShaderStageCreateInfos,         // stages
                    &pipelineVertexInputStateCreateInfo,    // pVertexInputState
                    &pipelineInputAssemblyStateCreateInfo,  // pInputAssemblyState
                    nullptr,                                // pTessellationState
                    &pipelineViewportStateCreateInfo,       // pViewportState
                    &pipelineRasterizationStateCreateInfo,  // pRasterizationState
                    &pipelineMultisampleStateCreateInfo,    // pMultisampleState
                    &pipelineDepthStencilStateCreateInfo,   // pDepthStencilState
                    &pipelineColorBlendStateCreateInfo,     // pColorBlendState
                    &pipelineDynamicStateCreateInfo,        // pDynamicState
                    out->pipelineLayout,                    // layout
                    renderPass                              // renderPass
            );

            vk::Result result;
            std::tie(result, out->pipeline) = m_Device.createGraphicsPipeline(nullptr, graphicsPipelineCreateInfo);
            switch (result) {
                case vk::Result::eSuccess:
                    break;
                case vk::Result::ePipelineCompileRequiredEXT:
                    Log::Core::Error("Pipeline compile required");
                    break;
                default:
                    break;
            }
        }

        vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(m_DescriptorPool, out->descriptorSetLayouts);
        out->descriptorSets = m_Device.allocateDescriptorSets(descriptorSetAllocateInfo);

        return out;
    }

    PipelineBuilder& PipelineBuilder::Clear() {
        m_VertexInputBindingDescription = vk::VertexInputBindingDescription();
        m_AttributeDescriptions.clear();

        for (auto& shader: m_VertexShaders) {
            m_Device.destroyShaderModule(shader);
        }
        for (auto& shader: m_FragmentShaders) {
            m_Device.destroyShaderModule(shader);
        }
        m_FragmentShaders.clear();
        m_VertexShaders.clear();
        m_PushConstants.clear();
        m_PushConstantOffset = 0u;
        m_DescriptorSetLayoutBindings.clear();

        return *this;
    }

    PipelineBuilder::~PipelineBuilder() {
        Clear();
        m_Device.destroyDescriptorPool(m_DescriptorPool);
    }

    std::optional<vk::ShaderModule> PipelineBuilder::loadShaderModule(std::string path) {
        std::ifstream file(path.data(), std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            Log::Core::Critical("Shader file not found: {}", path);
            return {};
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

        file.seekg(0);
        file.read((char*)buffer.data(), (std::streamsize)fileSize);
        file.close();

        return m_Device
                .createShaderModule(vk::ShaderModuleCreateInfo({}, buffer.size() * sizeof(uint32_t), buffer.data()));
    }

    PipelineBuilder::Pipeline::Pipeline(const vk::Device& device, const vk::DescriptorPool& descriptorPool,
                                        const std::map<uint32_t, std::map<uint32_t, vk::DescriptorSetLayoutBinding>>& descriptorSetLayoutBindings)
            : device(device), descriptorPool(descriptorPool),
              descriptorSetLayoutBindings(descriptorSetLayoutBindings) {}

    void PipelineBuilder::Pipeline::UpdateBuffer(uint32_t set, uint32_t binding, vk::DescriptorBufferInfo info,
                                                 uint32_t index) {
        vk::WriteDescriptorSet writeDescriptorSet(
                descriptorSets[set], binding, index, descriptorSetLayoutBindings.at(set).at(binding).descriptorType,
                {}, info);
        device.updateDescriptorSets(writeDescriptorSet, nullptr);
    }

    void PipelineBuilder::Pipeline::UpdateImage(uint32_t set, uint32_t binding, vk::DescriptorImageInfo info,
                                                uint32_t index) {
        vk::WriteDescriptorSet writeDescriptorSet(
                descriptorSets[set], binding, index, descriptorSetLayoutBindings.at(set).at(binding).descriptorType,
                info, {});
        device.updateDescriptorSets(writeDescriptorSet, nullptr);
    }

    PipelineBuilder::Pipeline::~Pipeline() {
        device.freeDescriptorSets(descriptorPool, descriptorSets);

        device.destroyPipeline(pipeline);
        device.destroyPipelineLayout(pipelineLayout);

        for (auto& layout: descriptorSetLayouts) {
            device.destroyDescriptorSetLayout(layout);
        }
    }
}