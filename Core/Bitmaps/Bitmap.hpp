#pragma once

#include "glm/glm.hpp"
#include <filesystem>

namespace MapleLeaf {
class Bitmap
{
public:
    Bitmap() = default;
    explicit Bitmap(std::filesystem::path filename);
    explicit Bitmap(const glm::uvec2& size, uint32_t bytesPerPixel = 4);
    Bitmap(std::unique_ptr<uint8_t[]>&& data, const glm::uvec2& size, uint32_t bytesPerPixel = 4);
    ~Bitmap() = default;

    void ConvertBGRAtoRGBA(uint8_t* bgraData, uint32_t width, uint32_t height) const;

    void Load(const std::filesystem::path& filename);
    void Write(const std::filesystem::path& filename) const;

    explicit operator bool() const noexcept { return !data; }


    uint32_t GetLength() const;

    const std::filesystem::path& GetFilename() const { return filename; }
    void                         SetFilename(const std::filesystem::path& filename) { this->filename = filename; }

    const std::unique_ptr<uint8_t[]>& GetData() const { return data; }
    std::unique_ptr<uint8_t[]>&       GetData() { return data; }
    void                              SetData(std::unique_ptr<uint8_t[]>&& data) { this->data = std::move(data); }

    const glm::uvec2& GetSize() const { return size; }
    void              SetSize(const glm::uvec2& size) { this->size = size; }

    uint32_t GetBytesPerPixel() const { return bytesPerPixel; }
    void     SetBytesPerPixel(uint32_t bytesPerPixel) { this->bytesPerPixel = bytesPerPixel; }

private:
    static uint32_t CalculateLength(const glm::uvec2& size, uint32_t bytesPerPixel);

    std::filesystem::path      filename;
    std::unique_ptr<uint8_t[]> data;
    glm::uvec2                 size;
    uint32_t                   bytesPerPixel = 0;
};
}   // namespace MapleLeaf