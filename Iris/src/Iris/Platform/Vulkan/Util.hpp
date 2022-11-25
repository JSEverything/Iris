#pragma once
#include <vulkan/vulkan.hpp>

void VkCheck(VkResult result, std::string_view msg);

void VkCheck(vk::Result result, std::string_view msg);

uint32_t findMemoryType(vk::PhysicalDeviceMemoryProperties const& memoryProperties, uint32_t typeBits,
                        vk::MemoryPropertyFlags requirementsMask);