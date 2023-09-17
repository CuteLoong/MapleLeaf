#pragma once

#include "IndirectBuffer.hpp"
#include "Shader.hpp"


namespace MapleLeaf {
class IndirectHandler
{
public:
    explicit IndirectHandler(bool multipipeline = false);
    explicit IndirectHandler(const Shader::UniformBlock& uniformBlock, bool multipipeline = false);

    void Push(void* data, std::size_t size)
    {
        if (this->size != size) {
            this->size    = static_cast<uint32_t>(size);
            handlerStatus = Buffer::Status::Reset;
            return;
        }

        if (!indirectBuffer) return;

        if (!bind) {
            indirectBuffer->MapMemory(&this->data);
            bind = true;
        }

        // If the buffer is already changed we can skip a memory comparison and just copy.
        if (handlerStatus == Buffer::Status::Changed || std::memcmp(static_cast<char*>(this->data), data, size) != 0) {
            std::memcpy(static_cast<char*>(this->data), data, size);
            handlerStatus = Buffer::Status::Changed;
        }
    }

    template<typename T>
    void Push(const T& object, std::size_t offset, std::size_t size)
    {
        if (!indirectBuffer) return;

        if (!bind) {
            indirectBuffer->MapMemory(&this->data);
            bind = true;
        }

        // If the buffer is already changed we can skip a memory comparison and just copy.
        if (handlerStatus == Buffer::Status::Changed || std::memcmp(static_cast<char*>(this->data) + offset, &object, size) != 0) {
            std::memcpy(static_cast<char*>(this->data) + offset, &object, size);
            handlerStatus = Buffer::Status::Changed;
        }
    }

    bool Update(const std::optional<Shader::UniformBlock>& uniformBlock);

    const IndirectBuffer* GetIndirectBuffer() const { return indirectBuffer.get(); }

private:
    bool                                multipipeline;
    // std::optional<Shader::UniformBlock> uniformBlock;
    uint32_t                            size = 0;
    void*                               data = nullptr;
    bool                                bind = false;
    std::unique_ptr<IndirectBuffer>     indirectBuffer;
    Buffer::Status                      handlerStatus;
};
}   // namespace MapleLeaf