#include "Bitmap.hpp"
#include "Files.hpp"
#include "Log.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stb_image_write.h>

namespace MapleLeaf {
Bitmap::Bitmap(std::filesystem::path filename)
    : filename(std::move(filename))
{
    Load(this->filename);
}

Bitmap::Bitmap(const glm::uvec2& size, uint32_t bytesPerPixel)
    : data(std::make_unique<uint8_t[]>(CalculateLength(size, bytesPerPixel)))
    , size(size)
    , bytesPerPixel(bytesPerPixel)
{}

Bitmap::Bitmap(std::unique_ptr<uint8_t[]>&& data, const glm::uvec2& size, uint32_t bytesPerPixel)
    : data(std::move(data))
    , size(size)
    , bytesPerPixel(bytesPerPixel)
{}

void Bitmap::Load(const std::filesystem::path& filename)
{
    auto pathStr = filename.string();
    std::replace(pathStr.begin(), pathStr.end(), '\\', '/');

    auto exisitPath = Files::Get()->GetExistPath(pathStr);
    if (!exisitPath) {
        Log::Error("Bitmap could not be loaded: ", filename, '\n');
        return;
    }

    data          = std::unique_ptr<uint8_t[]>(stbi_load(exisitPath->string().c_str(),
                                                reinterpret_cast<int32_t*>(&size.x),
                                                reinterpret_cast<int32_t*>(&size.y),
                                                reinterpret_cast<int32_t*>(&bytesPerPixel),
                                                STBI_rgb_alpha));
    bytesPerPixel = 4;
}

void Bitmap::ConvertBGRAtoRGBA(uint8_t* bgraData, uint32_t width, uint32_t height) const
{
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            uint32_t i = (y * width + x) * 4;
            std::swap(bgraData[i], bgraData[i + 2]);
        }
    }
}

void Bitmap::Write(const std::filesystem::path& filename) const
{
    if (auto parentPath = filename.parent_path(); !parentPath.empty()) std::filesystem::create_directories(parentPath);

    std::ofstream os(filename, std::ios::binary | std::ios::out);
    int32_t       len;

    // ConvertBGRAtoRGBA(data.get(), size.x, size.y);
    std::unique_ptr<uint8_t[]> png(stbi_write_png_to_mem(data.get(), size.x * bytesPerPixel, size.x, size.y, bytesPerPixel, &len));
    os.write(reinterpret_cast<char*>(png.get()), len);
}

uint32_t Bitmap::GetLength() const
{
    return size.x * size.y * bytesPerPixel;
}

uint32_t Bitmap::CalculateLength(const glm::uvec2& size, uint32_t bytesPerPixel)
{
    return size.x * size.y * bytesPerPixel;
}
}   // namespace MapleLeaf
