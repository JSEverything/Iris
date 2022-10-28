#pragma once
#include "Iris/Core/Base.hpp"
#include "Iris/Core/Application.hpp"

#ifdef IRIS_PLATFORM_LINUX
#include "Iris/Platform/Linux/EntryPoint.hpp"

#elifdef IRIS_PLATFORM_WINDOWS
#include "Iris/Platform/Windows/EntryPoint.hpp"

#endif

int AppMain(const std::vector<std::string_view>& args) {
    Iris::Log::Init();

    auto app = Iris::CreateApplication(args);

    app->Run();

    delete app;
    return 0;
}
