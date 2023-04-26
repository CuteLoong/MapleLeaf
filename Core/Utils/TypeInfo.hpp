#pragma once

#include <typeindex>
#include <unordered_map>

namespace MapleLeaf {
using TypeId = std::size_t;

template<typename T>
class TypeInfo
{
public:
    TypeInfo() = default;

    /**
     * Get the type ID of K which is a base of T.
     * @tparam K The type ID K.
     * @return The type ID.
     */
    template<typename K, typename = std::enable_if_t<std::is_convertible_v<K*, T*>>>
    static TypeId GetTypeId() noexcept
    {
        std::type_index typeIndex(typeid(K));
        if (auto it = typeMap.find(typeIndex); it != typeMap.end()) return it->second;

        const auto id      = NextTypeId();
        typeMap[typeIndex] = id;
        return id;
    }

private:
    // Next type ID for T.
    static TypeId                                      nextTypeId;
    static std::unordered_map<std::type_index, TypeId> typeMap;

    /**
     * Get the next type ID for T
     * @return The next type ID for T.
     */
    static TypeId NextTypeId() noexcept
    {
        const auto id = nextTypeId;
        ++nextTypeId;
        return id;
    }
};

template<typename T>
TypeId TypeInfo<T>::nextTypeId = 0;

template<typename T>
std::unordered_map<std::type_index, TypeId> TypeInfo<T>::typeMap = {};
}   // namespace MapleLeaf