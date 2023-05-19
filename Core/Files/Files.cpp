#include "Files.hpp"

#include "Log.hpp"
#include <exception>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>

namespace MapleLeaf {
Files::Files() {}
Files::~Files() {}

void Files::Update() {}

std::optional<std::string> Files::Read(const std::filesystem::path& path)
{
    auto pathStr = path.string();
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
}   // namespace MapleLeaf