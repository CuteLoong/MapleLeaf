#pragma once

#include "Material.hpp"
#include "Model.hpp"
#include "NonCopyable.hpp"
#include <filesystem>

namespace MapleLeaf {
class AssimpImporter : public NonCopyable
{
public:
    static void import(const std::filesystem::path& path, Model& model, Material& material);

private:
    AssimpImporter() = default;
};
}   // namespace MapleLeaf