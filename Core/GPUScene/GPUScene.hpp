#pragma once

#include "DescriptorHandler.hpp"
#include "GPUInstances.hpp"
#include "StorageHandler.hpp"
#include "UniformHandler.hpp"
#include "GPUUpdateInfos.hpp"

namespace MapleLeaf {
class GPUScene
{
    friend class Scene;

public:
    GPUScene();

    void Start();
    void Update();

    void PushDescriptors(DescriptorsHandler& descriptorSet);

private:
    std::unique_ptr<GPUInstances> gpuInstances;

    StorageHandler                        instanceDataHandler;
    std::vector<StorageHandler>           verticesHandlers;
    std::vector<StorageHandler>           indicesHandlers;
    std::vector<std::shared_ptr<Image2d>> images;

    std::vector<VkDrawIndirectCommand> drawCommands;

    bool started = false;

    std::shared_ptr<GPUUpdateInfos> updateInfos;
};
}   // namespace MapleLeaf