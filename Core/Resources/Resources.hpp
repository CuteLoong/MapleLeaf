#pragma once

#include "Engine.hpp"
#include "Files.hpp"
#include "Graphics.hpp"
#include "Resource.hpp"
#include "ThreadPool.hpp"
#include <unordered_map>

namespace MapleLeaf {
class Resources : public Module::Registrar<Resources>
{
    inline static const bool Registered =
        Register(Stage::Post, Requires<Files, Graphics>());   // Require Graphics, because model need free before vkDestroyDevice.

public:
    Resources();

    void Update() override;

    uint32_t GetResourceIndex(const std::shared_ptr<Resource>& resource) const;

    std::shared_ptr<Resource> Find(const std::type_index& typeIndex) const;

    const std::vector<std::shared_ptr<Resource>> FindAll(const std::type_index& typeIndex) const;

    template<typename T>
    std::shared_ptr<T> Find() const
    {
        if (auto resource = Find(typeid(T)); resource != nullptr)
            return std::static_pointer_cast<T>(resource);
        else
            return nullptr;
    }

    template<typename T>
    const std::vector<std::shared_ptr<T>> FindAll() const
    {
        if (auto resource = Find(typeid(T)); resource == nullptr) return {};
        std::vector<std::shared_ptr<T>> result;
        for (const auto& resource : FindAll(typeid(T))) {
            result.emplace_back(std::static_pointer_cast<T>(resource));
        }
        return result;
    }

    void Add(const std::shared_ptr<Resource>& resource);
    void Remove(const std::shared_ptr<Resource>& resource);

    /**
     * Gets the resource loader thread pool.
     * @return The resource loader thread pool.
     */
    ThreadPool& GetThreadPool() { return threadPool; }

private:
    std::unordered_map<std::type_index, std::vector<std::shared_ptr<Resource>>> resources;
    ElapsedTime                                                                 elapsedPurge;

    ThreadPool threadPool;
};
}   // namespace MapleLeaf