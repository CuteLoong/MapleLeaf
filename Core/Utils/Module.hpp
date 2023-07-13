#pragma once

#include "NonCopyable.hpp"
#include "TypeInfo.hpp"
#include <bitset>
#include <functional>
#include <memory>
#include <sys/stat.h>
#include <unordered_map>

namespace MapleLeaf {
template<typename Base>
class ModuleFactory
{
public:
    class TCreateValue
    {
    public:
        std::function<std::unique_ptr<Base>()> create;
        typename Base::Stage                   stage;
        std::vector<TypeId>                    requiremets;
    };
    using TRegistryMap = std::unordered_map<TypeId, TCreateValue>;

    virtual ~ModuleFactory() = default;

    static TRegistryMap& Registry()
    {
        static TRegistryMap impl;
        return impl;
    }

    template<typename... Args>
    class Requires
    {
    public:
        std::vector<TypeId> Get() const
        {
            std::vector<TypeId> requirements;
            (requirements.emplace_back(TypeInfo<Base>::template GetTypeId<Args>()), ...);
            return requirements;
        }
    };

    template<typename T>
    class Registrar : public Base
    {
    public:
        /**
         * Virtual deconstructor called from the engine to clear the instance pointer.
         */
        virtual ~Registrar()
        {
            if (static_cast<T*>(this) == moduleInstance) moduleInstance = nullptr;
        }

        /**
         * Gets the engines instance.
         * @return The current module instance.
         */
        static T* Get() { return moduleInstance; }

    protected:
        inline static T* moduleInstance = nullptr;

        /**
         * Creates a new module singleton instance and registers into the module registry map.
         * @tparam Args Modules that will be initialized before this module.
         * @return A dummy value in static initialization.
         */
        template<typename... Args>
        static bool Register(typename Base::Stage stage, Requires<Args...>&& requirements = {})
        {
            ModuleFactory::Registry()[TypeInfo<Base>::template GetTypeId<T>()] = {[]() {
                                                                                      moduleInstance = new T();
                                                                                      // The registrar does not own the instance, the engine does, we
                                                                                      // just hold a raw pointer for convenience.
                                                                                      return std::unique_ptr<Base>(moduleInstance);
                                                                                  },
                                                                                  stage,
                                                                                  requirements.Get()};
            return true;
        }
    };
};

class Module : public ModuleFactory<Module>, NonCopyable
{
public:
    /**
     * @brief Represents the stage where the module will be updated in the engine.
     */
    enum class Stage : uint8_t
    {
        Never,
        Always,
        Pre,
        Normal,
        Post,
        Render
    };

    using StageIndex = std::pair<Stage, TypeId>;

    Module() { TypeInfo<Module>::GetTypeId<Module>(); }
    virtual ~Module() = default;

    /**
     * The update function for the module.
     */
    virtual void Update() = 0;
};

class ModuleFilter
{
public:
    // Include all modules by default.
    ModuleFilter() { include.set(); }

    template<typename T>
    bool Check() const noexcept
    {
        return include.test(TypeInfo<Module>::GetTypeId<T>());
    }

    bool Check(TypeId typeId) const noexcept { return include.test(typeId); }

    template<typename T>
    ModuleFilter& Exclude() noexcept
    {
        include.reset(TypeInfo<Module>::GetTypeId<T>());
        return *this;
    }

    template<typename T>
    ModuleFilter& Include() noexcept
    {
        include.set(TypeInfo<Module>::GetTypeId<T>());
        return *this;
    }

    ModuleFilter& ExcludeAll() noexcept
    {
        include.reset();
        return *this;
    }

    ModuleFilter& IncludeAll() noexcept
    {
        include.set();
        return *this;
    }

private:
    std::bitset<64> include;
};
}   // namespace MapleLeaf