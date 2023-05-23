#pragma once

#include <memory>

namespace MapleLeaf {
// TODO C++20: std::to_address
template<typename T>
static T* to_address(T* obj) noexcept
{
    return obj;
}

template<typename T>
static T* to_address(T& obj) noexcept
{
    return &obj;
}

template<typename T>
static T* to_address(const std::shared_ptr<T>& obj) noexcept
{
    return obj.get();
}

template<typename T>
static T* to_address(const std::unique_ptr<T>& obj) noexcept
{
    return obj.get();
}

template<typename T>
static const T& to_reference(T& obj) noexcept
{
    return obj;
}

template<typename T>
static const T& to_reference(T* obj) noexcept
{
    return *obj;
}

template<typename T>
static const T& to_reference(const std::shared_ptr<T>& obj) noexcept
{
    return *obj.get();
}

template<typename T>
static const T& to_reference(const std::unique_ptr<T>& obj) noexcept
{
    return *obj.get();
}
}   // namespace MapleLeaf