#pragma once
#include <vector>
#include <string_view>

extern int AppMain(const std::vector<std::string_view>& args);

int main(int argc, char** argv) {
    std::vector<std::string_view> args;
    for (int i = 0; i < argc; ++i) {
        args.emplace_back(std::string_view(argv[i]));
    }
    
    return AppMain(args);
}