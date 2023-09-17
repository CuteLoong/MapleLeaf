#include "IndirectHandler.hpp"

namespace MapleLeaf {
IndirectHandler::IndirectHandler(bool multipipeline)
    : multipipeline(multipipeline)
    , handlerStatus(Buffer::Status::Reset)
{}

IndirectHandler::IndirectHandler(const Shader::UniformBlock& uniformBlock, bool multipipeline)
    : multipipeline(multipipeline)
    , size(static_cast<uint32_t>(0))
    , indirectBuffer(std::make_unique<IndirectBuffer>(static_cast<VkDeviceSize>(size)))
    , handlerStatus(Buffer::Status::Changed)
{}

bool IndirectHandler::Update(const std::optional<Shader::UniformBlock>& uniformBlock)
{
    if (handlerStatus == Buffer::Status::Reset) {
        if (size == 0) {
            size = static_cast<uint32_t>(uniformBlock->GetSize());
        }
        bind           = false;
        indirectBuffer = std::make_unique<IndirectBuffer>(static_cast<VkDeviceSize>(size));
        handlerStatus  = Buffer::Status::Changed;
        return false;
    }

    if (handlerStatus != Buffer::Status::Normal) {
        if (bind) {
            indirectBuffer->UnmapMemory();
            bind = false;
        }

        handlerStatus = Buffer::Status::Normal;
    }

    return true;
}
}   // namespace MapleLeaf