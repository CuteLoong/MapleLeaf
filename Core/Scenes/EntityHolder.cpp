#include "EntityHolder.hpp"

namespace MapleLeaf {
EntityHolder::EntityHolder() {}

void EntityHolder::Update()
{
    for (auto it = entities.begin(); it != entities.end();) {
        if ((*it)->IsRemoved()) {
            it = entities.erase(it);
            continue;
        }

        (*it)->Update();
        ++it;
    }
}

Entity* EntityHolder::GetEntity(const std::string& name) const
{
    for (auto& entity : entities) {
        if (entity->GetName() == name) return entity.get();
    }

    return nullptr;
}

Entity* EntityHolder::CreateEntity()
{
    return entities.emplace_back(std::make_unique<Entity>()).get();
}

void EntityHolder::Add(std::unique_ptr<Entity>&& entity)
{
    entities.emplace_back(std::move(entity));
}

void EntityHolder::Remove(Entity* entity)
{
    entities.erase(std::remove_if(entities.begin(), entities.end(), [entity](const auto& e) { return e.get() == entity; }), entities.end());
}

void EntityHolder::Move(Entity* entity, EntityHolder& structure)
{
    for (auto it = --entities.end(); it != entities.begin(); --it) {
        if ((*it).get() != entity) continue;

        structure.Add(std::move(*it));
        entities.erase(it);
    }
}

void EntityHolder::Clear()
{
    entities.clear();
}

std::vector<Entity *> EntityHolder::QueryAll() {
	std::vector<Entity *> entities;

	for (const auto &entity : this->entities) {
		if (entity->IsRemoved())
			continue;

		entities.emplace_back(entity.get());
	}

	return entities;
}

bool EntityHolder::Contains(Entity* entity)
{
    for (const auto& tmpEntity : entities) {
        if (tmpEntity.get() == entity) return true;
    }

    return false;
}
}   // namespace MapleLeaf