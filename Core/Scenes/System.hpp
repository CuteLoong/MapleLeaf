#pragma once

#include "NonCopyable.hpp"
#include "TypeInfo.hpp"

namespace MapleLeaf {
class System : NonCopyable
{
public:
    virtual ~System() = default;

    virtual void Update() = 0;

    bool IsEnabled() const { return enabled; }
    void SetEnabled(bool enable) { this->enabled = enable; }

private:
    bool enabled = true;
};

template class TypeInfo<System>;
}   // namespace MapleLeaf