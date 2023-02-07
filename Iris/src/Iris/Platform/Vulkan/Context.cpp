#include "Context.hpp"
#include <GLFW/glfw3.h>
#include "Iris/Core/Log.hpp"
#include "Iris/Platform/Vulkan/Util.hpp"

namespace Iris::Vulkan {
    Context::Context(const std::shared_ptr<Window>& window) {
        CreateInstance();
        CreateSurface(window);
        SelectDevice();
        CreateQueues();
    }

    void Context::CreateInstance() {
        vkb::InstanceBuilder instanceBuilder;

        auto instance = instanceBuilder
                .set_app_name("Iris")
                .set_engine_name("Iris")
                .set_debug_callback([](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                       VkDebugUtilsMessageTypeFlagsEXT messageType,
                                       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                       void* pUserData) -> VkBool32 {
                                        auto type = vkb::to_string_message_type(messageType);
                                        auto msg = std::string_view(pCallbackData->pMessage);

                                        switch (messageSeverity) {
                                            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                                                Log::Core::Error("{}: {}", type, msg);
                                                break;
                                            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                                                Log::Core::Warn("{}: {}", type, msg);
                                                break;
                                            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                                                Log::Core::Info("{}: {}", type, msg);
                                                break;
                                            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                                                Log::Core::Trace("{}: {}", type, msg);
                                                break;
                                            default:
                                                break;
                                        }
                                        return VK_FALSE;
                                    }
                )
                .request_validation_layers()
                .require_api_version(1, 3)
                .build();
        if (!instance) {
            Log::Core::Critical("Failed to create Vulkan instance. Error: {}\n", instance.error().message());
            std::exit(1);
        }
        m_VKBInstance = instance.value();
        m_Instance = m_VKBInstance;
    }

    void Context::CreateSurface(const std::shared_ptr<Window>& window) {
        if (window) {
            glfwMakeContextCurrent(window->GetGLFWWindow());
            VkSurfaceKHR surface;
            VkCheck(glfwCreateWindowSurface(m_Instance, window->GetGLFWWindow(), nullptr, &surface),
                    "glfwCreateWindowSurface");
            m_Surface = vk::SurfaceKHR(surface);
        } else {
            // Headless
        }
    }

    void Context::SelectDevice() {
        VkPhysicalDeviceVulkan12Features features12{};
        features12.runtimeDescriptorArray = true;
        features12.descriptorBindingPartiallyBound = true;

        features12.shaderUniformBufferArrayNonUniformIndexing = true;
        features12.shaderStorageBufferArrayNonUniformIndexing = true;
        features12.shaderSampledImageArrayNonUniformIndexing = true;
        features12.shaderStorageImageArrayNonUniformIndexing = true;

        //features12.descriptorBindingUniformBufferUpdateAfterBind = true;
        features12.descriptorBindingStorageBufferUpdateAfterBind = true;
        features12.descriptorBindingSampledImageUpdateAfterBind = true;
        features12.descriptorBindingStorageImageUpdateAfterBind = true;

        vkb::PhysicalDeviceSelector selector{ m_VKBInstance };
        auto phys = selector
                .set_surface(m_Surface)
                .set_minimum_version(1, 3)
                .require_dedicated_transfer_queue()
                .set_required_features_12(features12)
                .select();
        if (!phys) {
            Log::Core::Critical("Failed to select Vulkan Physical Device. Error: {}\n", phys.error().message());
            std::exit(1);
        }
        m_PhysicalDevice = vk::PhysicalDevice(phys.value());

        vkb::DeviceBuilder device_builder{ phys.value() };
        auto dev = device_builder.build();
        if (!dev) {
            Log::Core::Critical("Failed to select Vulkan Device. Error: {}\n", dev.error().message());
            std::exit(1);
        }
        m_VKBDevice = dev.value();
        m_Device = vk::Device(m_VKBDevice);
    }

    void Context::CreateQueues() {
        auto gq = m_VKBDevice.get_queue(vkb::QueueType::graphics);
        auto gqi = m_VKBDevice.get_queue_index(vkb::QueueType::graphics);
        if (!gq.has_value() || !gqi.has_value()) {
            Log::Core::Error("failed to get graphics queue: {}", gq.error().message());
            exit(1);
        }
        m_GraphicsQueue = gq.value();
        m_GraphicsQueueFamilyIndex = gqi.value();

        auto pq = m_VKBDevice.get_queue(vkb::QueueType::present);
        auto pqi = m_VKBDevice.get_queue_index(vkb::QueueType::present);
        if (!pq.has_value() || !pqi.has_value()) {
            Log::Core::Error("failed to get present queue: {}", pq.error().message());
            exit(1);
        }
        m_PresentQueue = pq.value();
        m_PresentQueueFamilyIndex = pqi.value();

        auto cq = m_VKBDevice.get_queue(vkb::QueueType::compute);
        auto cqi = m_VKBDevice.get_queue_index(vkb::QueueType::compute);
        if (!cq.has_value() || !cqi.has_value()) {
            Log::Core::Error("failed to get compute queue: {}", cq.error().message());
            exit(1);
        }
        m_ComputeQueue = cq.value();
        m_ComputeQueueFamilyIndex = cqi.value();

        auto tq = m_VKBDevice.get_queue(vkb::QueueType::transfer);
        auto tqi = m_VKBDevice.get_queue_index(vkb::QueueType::transfer);
        if (!tq.has_value() || !tqi.has_value()) {
            Log::Core::Error("failed to get transfer queue: {}", tq.error().message());
            exit(1);
        }
        m_TransferQueue = tq.value();
        m_TransferQueueFamilyIndex = tqi.value();
    }

    Context::~Context() {
        vkb::destroy_device(m_VKBDevice);
        vkb::destroy_surface(m_VKBInstance, m_Surface);
        vkb::destroy_instance(m_VKBInstance);
    }

    uint32_t Context::GetGraphicsQueueFamilyIndex() const {
        return m_GraphicsQueueFamilyIndex;
    }

    uint32_t Context::GetComputeQueueFamilyIndex() const {
        return m_ComputeQueueFamilyIndex;
    }

    uint32_t Context::GetPresentQueueFamilyIndex() const {
        return m_PresentQueueFamilyIndex;
    }

    uint32_t Context::GetTransferQueueFamilyIndex() const {
        return m_TransferQueueFamilyIndex;
    }

    const vk::Queue& Context::GetGraphicsQueue() const {
        return m_GraphicsQueue;
    }

    const vk::Queue& Context::GetComputeQueue() const {
        return m_ComputeQueue;
    }

    const vk::Queue& Context::GetPresentQueue() const {
        return m_PresentQueue;
    }

    const vk::Queue& Context::GetTransferQueue() const {
        return m_TransferQueue;
    }
}
