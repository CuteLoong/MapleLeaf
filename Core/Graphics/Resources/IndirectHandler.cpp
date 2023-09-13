#include "IndirectHandler.hpp"

namespace MapleLeaf {
IndirectHandler::IndirectHandler(bool multipipeline)
    : multipipeline(multipipeline)
    , handlerStatus(Buffer::Status::Reset)
{}

IndirectHandler::IndirectHandler(const Shader::UniformBlock& uniformBlock, bool multipipeline)
    : multipipeline(multipipeline)
    , uniformBlock(uniformBlock)
    , size(static_cast<uint32_t>(this->uniformBlock->GetSize()))
    , storageBuffer(std::make_unique<IndirectBuffer>(static_cast<VkDeviceSize>(size)))
    , handlerStatus(Buffer::Status::Changed)
{}

bool IndirectHandler::Update(const std::optional<Shader::UniformBlock>& uniformBlock)
{
    if (handlerStatus == Buffer::Status::Reset || multipipeline && !this->uniformBlock || !multipipeline && this->uniformBlock != uniformBlock) {
        if (size == 0 && !this->uniformBlock ||
            this->uniformBlock && this->uniformBlock != uniformBlock && static_cast<uint32_t>(this->uniformBlock->GetSize()) == size) {
            size = static_cast<uint32_t>(uniformBlock->GetSize());
        }

        this->uniformBlock = uniformBlock;
        bind               = false;
        storageBuffer      = std::make_unique<IndirectBuffer>(static_cast<VkDeviceSize>(size));
        handlerStatus      = Buffer::Status::Changed;
        return false;
    }

    if (handlerStatus != Buffer::Status::Normal) {
        if (bind) {
            storageBuffer->UnmapMemory();
            bind = false;
        }

        handlerStatus = Buffer::Status::Normal;
    }

    return true;
}
}   // namespace MapleLeaf