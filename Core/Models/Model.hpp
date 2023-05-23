#pragma once

#include "Buffer.hpp"
#include "Resource.hpp"
#include "glm/glm.hpp"
#include <filesystem>
#include <functional>
#include <memory>

namespace MapleLeaf {
template<typename Base>
class ModelFactory
{
public:
    using TCreateReturn = std::shared_ptr<Base>;

    using TCreateMethodFilename = std::function<TCreateReturn(const std::filesystem::path&)>;
    using TRegistryMapFilename  = std::unordered_map<std::string, TCreateMethodFilename>;

    virtual ~ModelFactory() = default;

    static TRegistryMapFilename& RegistryFilename()
    {
        static TRegistryMapFilename impl;
        return impl;
    }

    static TCreateReturn Create(const std::filesystem::path& filename)
    {
        auto fileExt = filename.extension().string();
        auto it      = RegistryFilename().find(fileExt);
        return it == RegistryFilename().end() ? nullptr : it->second(filename);
    }

    template<typename T>
    class Registrar : public Base
    {
    protected:
        static bool Register(const std::string& extension)
        {
            ModelFactory::RegistryFilename()[extension] = [](const std::filesystem::path& filename) -> TCreateReturn { return T::Create(filename); };
            return true;
        }
    };
    inline static std::string name;
};

class Model : public ModelFactory<Model>, public Resource
{
public:
    /**
     * Creates a new empty model.
     */
    Model() = default;

    /**
     * Creates a new model.
     * @tparam T The vertex type.
     * @param vertices The model vertices.
     * @param indices The model indices.
     */
    template<typename T>
    explicit Model(const std::vector<T>& vertices, const std::vector<uint32_t>& indices = {});

    bool CmdRender(const CommandBuffer& commandBuffer, uint32_t instances = 1) const;

    std::type_index GetTypeIndex() const override { return typeid(Model); }

    template<typename T>
    std::vector<T> GetVertices(std::size_t offset = 0) const;
    template<typename T>
    void SetVertices(const std::vector<T>& vertices);

    std::vector<uint32_t> GetIndices(std::size_t offset = 0) const;
    void                  SetIndices(const std::vector<uint32_t>& indices);

    const glm::vec3&   GetMinExtents() const { return minExtents; }
    const glm::vec3&   GetMaxExtents() const { return maxExtents; }
    float              GetWidth() const { return maxExtents.x - minExtents.x; }
    float              GetHeight() const { return maxExtents.y - minExtents.y; }
    float              GetDepth() const { return maxExtents.z - minExtents.z; }
    float              GetRadius() const { return radius; }
    const Buffer*      GetVertexBuffer() const { return vertexBuffer.get(); }
    const Buffer*      GetIndexBuffer() const { return indexBuffer.get(); }
    uint32_t           GetVertexCount() const { return vertexCount; }
    uint32_t           GetIndexCount() const { return indexCount; }
    static VkIndexType GetIndexType() { return VK_INDEX_TYPE_UINT32; }

protected:
    template<typename T>
    void Initialize(const std::vector<T>& vertices, const std::vector<uint32_t>& indices = {});

private:
    std::unique_ptr<Buffer> vertexBuffer;
    std::unique_ptr<Buffer> indexBuffer;
    uint32_t                vertexCount = 0;
    uint32_t                indexCount  = 0;

    glm::vec3 minExtents;
    glm::vec3 maxExtents;
    float     radius = 0.0f;
};
}   // namespace MapleLeaf