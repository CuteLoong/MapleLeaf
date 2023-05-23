#pragma once

#include "Log.hpp"
#include "TypeInfo.hpp"
#include <functional>


namespace MapleLeaf {
template<typename Base, class... Args>
class StreamFactory
{
public:
    using TCreateReturn = std::unique_ptr<Base>;

    using TCreateMethod = std::function<TCreateReturn(Args...)>;
    using TRegistryMap  = std::unordered_map<std::string, TCreateMethod>;

    virtual ~StreamFactory() = default;

    static TCreateReturn Create(const std::string& name, Args&&... args)
    {
        auto it = Registry().find(name);
        if (it == Registry().end()) {
            Log::Error("Failed to create ", std::quoted(name), " from factory\n");
            return nullptr;
        }
        return it->second(std::forward<Args>(args)...);
    }

    static TRegistryMap& Registry()
    {
        static TRegistryMap impl;
        return impl;
    }

    template<typename T>
    class Registrar : public Base
    {
    public:
        TypeId      GetTypeId() const override { return TypeInfo<Base>::template GetTypeId<T>(); }
        std::string GetTypeName() const override { return name; }

    protected:
        static bool Register(const std::string& name)
        {
            Registrar::name                 = name;
            StreamFactory::Registry()[name] = [](Args... args) -> TCreateReturn { return std::make_unique<T>(std::forward<Args>(args)...); };
            return true;
        }

        inline static std::string name;
    };

    virtual TypeId      GetTypeId() const { return -1; }
    virtual std::string GetTypeName() const { return ""; }
};
}   // namespace MapleLeaf