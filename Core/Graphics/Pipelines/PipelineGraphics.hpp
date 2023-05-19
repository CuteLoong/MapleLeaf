#pragma once

#include "Image2d.hpp"
#include "ImageDepth.hpp"
#include "Pipeline.hpp"
#include "RenderStage.hpp"
#include "glm/glm.hpp"
#include <vector>


namespace MapleLeaf {
class ImageDepth;
class Image2d;

class PipelineGraphics : public Pipeline
{
public:
    enum class Mode
    {
        Polygon,
        MRT
    };

    enum class Depth
    {
        None      = 0,
        Read      = 1,
        Write     = 2,
        ReadWrite = Read | Write
    };

private:
    std::vector<std::filesystem::path> shaderStages;
    std::vector<Shader::VertexInput>   vertexInputs;
};
}   // namespace MapleLeaf