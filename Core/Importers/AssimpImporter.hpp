#pragma once

#include "NonCopyable.hpp"
#include "Scenes.hpp"
#include <filesystem>
#include <vector>


namespace MapleLeaf {
class AssimpImporter : public NonCopyable
{
public:
    static void import(const std::filesystem::path& path, SceneBuilder& builder);

private:
    AssimpImporter() = default;
};
}   // namespace MapleLeaf