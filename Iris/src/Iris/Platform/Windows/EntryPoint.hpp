#pragma once
#include <windows.h>
#include <vector>
#include <string_view>

extern int AppMain(const std::vector<std::string_view>& args);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
    std::string str(lpCmdLine);

    auto prev = str.begin();
    uint64_t offset;

    std::vector<std::string_view> args;
    while ((offset = str.find(' ')) != std::string::npos) {
        args.emplace_back(std::string_view(prev, prev + (int64_t)offset));
        prev += (int64_t)offset;
    }

    return AppMain(args);
}