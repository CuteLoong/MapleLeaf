#pragma once

#include "NonCopyable.hpp"
#include <typeindex>

namespace MapleLeaf {
class Resource : NonCopyable
{
public:
    Resource()          = default;
    virtual ~Resource() = default;

    virtual std::type_index GetTypeIndex() const = 0;
};
}   // namespace MapleLeaf
