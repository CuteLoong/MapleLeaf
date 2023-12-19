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

uint32_t Resources::GetResourceIndex(const std::shared_ptr<Resource>& r) const
{
    if (resources.find(r->GetTypeIndex()) == resources.end()) return 0;

    uint32_t index = 0;
    for (const auto& resource : resources.at(r->GetTypeIndex())) {
        if (r == resource) return index;
        index++;
    }

    return 0;
}

std::shared_ptr<Resource> Resources::Find(const std::type_index& typeIndex) const
{
    if (resources.find(typeIndex) == resources.end()) return nullptr;

    for (const auto& resource : resources.at(typeIndex)) {
        return resource;
    }

    return nullptr;
}

const std::vector<std::shared_ptr<Resource>> Resources::FindAll(const std::type_index& typeIndex) const
{
    if (resources.find(typeIndex) == resources.end()) return {};
    return resources.at(typeIndex);
}

void Resources::Add(const std::shared_ptr<Resource>& resource)
{
    if (Find(resource->GetTypeIndex()) == resource) return;

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