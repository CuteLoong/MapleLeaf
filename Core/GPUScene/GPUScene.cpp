#include "GPUScene.hpp"
#include "Vertex.hpp"

// #define MAPLELEAF_DEBUG

namespace MapleLeaf {
GPUScene::GPUScene()
    : gpuInstances(std::make_unique<GPUInstances>())
{}

void GPUScene::Start()
{
    gpuInstances->Start();

    // create model storage infomation
    verticesHandlers.resize(gpuInstances->models.size());
    indicesHandlers.resize(gpuInstances->models.size());
    for (const auto& model : gpuInstances->models) {
        uint32_t modelIndex = model.second.first;

        verticesHandlers[modelIndex].Push(const_cast<Vertex3D*>(model.first->GetVertices().data()),
                                          sizeof(Vertex3D) * model.first->GetVertices().size());
        indicesHandlers[modelIndex].Push(const_cast<uint32_t*>(model.first->GetIndices().data()),
                                         sizeof(uint32_t) * model.first->GetIndices().size());
    }

    std::vector<GPUInstances::GPUInstanceData> instanceDataVector;
    for (auto& [mesh, instanceData] : gpuInstances->instanceDatas) instanceDataVector.push_back(instanceData);
    instanceDataHandler.Push(instanceDataVector.data(), sizeof(GPUInstances::GPUInstanceData) * instanceDataVector.size());

    // image directly push to descriptor
    for (const auto& [image, indexCount] : gpuInstances->images) {
        images.push_back(image);
    }
}

void GPUScene::Update()
{
    gpuInstances->Update();

#ifdef MAPLELEAF_DEBUG
    auto debugStart = Time::Now();
#endif
    // for (const auto& model : gpuInstances->models) {
    //     uint32_t modelIndex = model.second.first;

    //     verticesHandlers[modelIndex].Push(const_cast<Vertex3D*>(model.first->GetVertices().data()),
    //                                       sizeof(Vertex3D) * model.first->GetVertices().size());
    //     indicesHandlers[modelIndex].Push(const_cast<uint32_t*>(model.first->GetIndices().data()),
    //                                      sizeof(uint32_t) * model.first->GetIndices().size());
    // }

#ifdef MAPLELEAF_DEBUG
    Log::Out("Push Models costs: ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
    debugStart = Time::Now();
#endif
    for (const auto& mesh : gpuInstances->updatedMesh) {}
}

void GPUScene::PushDescriptors(DescriptorsHandler& descriptorSet)
{
    for (StorageHandler& vertices : verticesHandlers) {
        descriptorSet.Push("VerticesBuffers", vertices);
    }

    for (StorageHandler& indices : indicesHandlers) {
        descriptorSet.Push("IndicesBuffers", indices);
    }

    for (const auto& image : images) {
        descriptorSet.Push("ImageSamplers", image);
    }

    descriptorSet.Push("InstanceDatas", instanceDataHandler);
}
}   // namespace MapleLeaf