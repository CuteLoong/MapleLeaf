#pragma once

#include "Engine.hpp"
#include <filesystem>
#include <optional>

namespace MapleLeaf {
enum class FileMode
{
    Read,
    Write,
    Append
};

class Files : public Module::Registrar<Files>
{
    inline static const bool Registered = Register(Stage::Post);

public:
    Files();
    ~Files();

    void Update() override;

    /**
     * Gets if the path is found in one of the search paths.
     * @param path The path to look for.
     * @return If the path is found in one of the searches.
     */
    static bool ExistsInPath(const std::filesystem::path& path);

    /**
     * Reads a file found by real or partial path.
     * @param path The path to read.
     * @return The data read from the file.
     */
    static std::optional<std::string> Read(const std::filesystem::path& path);

private:
    std::vector<std::string> searchPaths;
};
}   // namespace MapleLeaf