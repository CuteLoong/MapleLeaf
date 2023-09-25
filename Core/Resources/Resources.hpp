#pragma once

#include "Engine.hpp"
#include "Files.hpp"
#include "Resource.hpp"
#include "ThreadPool.hpp"
#include <unordered_map>

namespace MapleLeaf {
class Resources : public Module::Registrar<Resources>
{
    inline static const bool Registered = Register(Stage::Post, Requires<Files>());

public:
    Resources();

    void Update() override;

    std::shared_ptr<Resource> Find(const std::type_index &typeIndex) const;

    void Add(const std::shared_ptr<Resource> &resource);
	void Remove(const std::shared_ptr<Resource> &resource);

    /**
     * Gets the resource loader thread pool.
     * @return The resource loader thread pool.
     */
    ThreadPool& GetThreadPool() { return threadPool; }

private:
    std::unordered_map<std::type_index, std::vector<std::shared_ptr<Resource>>> resources;
    ElapsedTime elapsedPurge;

    ThreadPool threadPool;
};
}   // namespace MapleLeaf