#include "Files.hpp"

#include "Log.hpp"
#include <exception>
#include <filesystem>

#include "config.h"

namespace MapleLeaf {
Files::Files()
    : rootPath(CONFIG_PROJECT_DIR)
{}
Files::~Files() {}

void Files::Update() {}

void Files::AddSearchPath(const std::string& path)
{
    if (std::find(searchPaths.begin(), searchPaths.end(), path) != searchPaths.end()) {
        return;
    }
    searchPaths.push_back(path);
}

void Files::RemoveSearchPath(const std::string& path)
{
    auto it = std::find(searchPaths.begin(), searchPaths.end(), path);
    if (it == searchPaths.end()) return;

    searchPaths.erase(it);
}

void Files::ClearSearchPath()
{
    searchPaths.clear();
}

std::optional<std::string> Files::Read(const std::filesystem::path& path)
{
    auto exisitPath = GetExistPath(path);
    if (!exisitPath) {
        Log::Error("Path ", path, "not found");
        return std::nullopt;
    }

    auto pathStr = exisitPath->string();
    std::replace(pathStr.begin(), pathStr.end(), '\\', '/');

    std::ifstream     fsFile;
    std::stringstream buffer;
    fsFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        fsFile.open(pathStr);
        if (!fsFile.is_open()) {
            if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path)) {
                throw std::exception("file is not exist or is not a regular file!");
            }
        }
        buffer << fsFile.rdbuf();

        fsFile.close();
    }
    catch (const std::exception& e) {
        Log::Error("Failed to open file ", path, ", exception is ", e.what());
        return std::nullopt;
    }
    return buffer.str();
}

bool Files::ExistsInPath(const std::filesystem::path& path)
{
    return std::filesystem::exists(path);
}

const std::optional<std::filesystem::path> Files::GetExistPath(const std::filesystem::path& path)
{
    if (ExistsInPath(path)) return path;
    for (const auto& searchPath : searchPaths) {
        if (ExistsInPath(rootPath / searchPath / path)) return rootPath / searchPath / path;
    }
    return std::nullopt;
}
}   // namespace MapleLeaf