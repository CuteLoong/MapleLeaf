#include "Resources.hpp"

namespace MapleLeaf {
Resources::Resources()
    : elapsedPurge(5s)
{}

void Resources::Update()
{
    if (elapsedPurge.GetElapsed() != 0) {
        for (auto it = resources.begin(); it != resources.end();) {
            for (auto it1 = it->second.begin(); it1 != it->second.end();) {
                if (it1->use_count() <= 1) {
                    it1 = it->second.erase(it1);
                    continue;
                }
                it1++;
            }

            if (it->second.empty()) {
                it = resources.erase(it);
                continue;
            }
            it++;
        }
    }
}

std::shared_ptr<Resource> Resources::Find(const std::type_index& typeIndex) const
{
    if (resources.find(typeIndex) == resources.end()) return nullptr;

    for (const auto& resource : resources.at(typeIndex)) {
        return resource;
    }

    return nullptr;
}

void Resources::Add(const std::shared_ptr<Resource>& resource)
{
    if (Find(resource->GetTypeIndex())) return;

    resources[resource->GetTypeIndex()].emplace_back(resource);
}

void Resources::Remove(const std::shared_ptr<Resource>& resource)
{
    auto& resources = this->resources[resource->GetTypeIndex()];
    for (auto it = resources.begin(); it != resources.end(); ++it) {
        if ((*it) == resource) resources.erase(it);
    }
    if (resources.empty()) this->resources.erase(resource->GetTypeIndex());
}
}   // namespace MapleLeaf