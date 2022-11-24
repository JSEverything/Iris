#pragma once
#include "Iris/Core/Log.hpp"

static void VkCheck(VkResult result, std::string_view msg) {
    if (result == VK_SUCCESS) return;
    //Iris::Log::Core::Error("VkResult Error: {}\n", msg);
    std::exit(1);
}