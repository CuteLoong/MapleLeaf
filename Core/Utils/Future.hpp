#pragma once

#include <future>
#include <optional>

namespace MapleLeaf {
template<typename T>
class Future
{
public:
    Future() noexcept = default;

    Future(std::future<T>&& future) noexcept
        : future(std::move(future))
    {}

    bool has_value() const noexcept { return future.valid() || current; }

    // will wait for result
    T& get() noexcept
    {
        if (future.valid()) {
            current = future.get();
        }

        return *current;
    }

    constexpr explicit operator bool() const noexcept { return has_value(); }
    constexpr operator T&() const noexcept { return *get(); }

    T& operator*() noexcept { return get(); }
    T& operator->() noexcept { return get(); }

    bool operator==(const Future& rhs) const noexcept { return future == rhs.future && current == rhs.current; }

    bool operator!=(const Future& rhs) const noexcept { return !operator==(rhs); }

private:
    std::future<T>   future;
    std::optional<T> current;
};
}   // namespace MapleLeaf