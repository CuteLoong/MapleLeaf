#include "DefaultBuilder.hpp"

namespace MapleLeaf {
Model* Builder::AddModel(const std::string name, std::shared_ptr<Model>&& model)
{
    if (models.count(name) != 0) Log::Info(name, " mesh replicate, please use instance draw!");

    models[name] = std::move(model);
    return models[name].get();
}

Model* Builder::GetModel(const std::string name)
{
    if (auto it = models.find(name); it != models.end() && it->second) return static_cast<Model*>(it->second.get());

    Log::Error("Models can't find model, which name is ", name);
    return nullptr;
}
}   // namespace MapleLeaf