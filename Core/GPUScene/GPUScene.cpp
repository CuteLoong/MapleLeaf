#include "GPUScene.hpp"
#include "Vertex.hpp"

// #define MAPLELEAF_DEBUG

namespace MapleLeaf {
GPUScene::GPUScene()
    : updateInfos(std::make_shared<GPUUpdateInfos>())
{
    gpuInstances = std::make_unique<GPUInstances>(updateInfos);
}

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
    const auto& updatedModels    = updateInfos->GetUpdatedModels();
    bool        clearModelUpdate = true;

    for (const auto& model : updatedModels) {
        uint32_t modelIndex = gpuInstances->models[model].first;
        verticesHandlers[modelIndex].Push(const_cast<Vertex3D*>(model->GetVertices().data()), sizeof(Vertex3D) * model->GetVertices().size());
        indicesHandlers[modelIndex].Push(const_cast<uint32_t*>(model->GetIndices().data()), sizeof(uint32_t) * model->GetIndices().size());

        // can resolve update problem, but need reconstruction TODO.
        clearModelUpdate &= (verticesHandlers[modelIndex].GetStorageBuffer() != nullptr);
        clearModelUpdate &= (indicesHandlers[modelIndex].GetStorageBuffer() != nullptr);
    }

    if (clearModelUpdate) updateInfos->ClearModelUpdates();


#ifdef MAPLELEAF_DEBUG
    Log::Out("Push Models costs: ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
    debugStart = Time::Now();
#endif
    // for (const auto& mesh : gpuInstances->updatedMeshDatas) {}
}

void GPUScene::PushDescriptors(DescriptorsHandler& descriptorSet)
{
    for (int i = 0; i < verticesHandlers.size(); i++) {
        descriptorSet.Push("VerticesBuffers", verticesHandlers[i], i);
        descriptorSet.Push("IndicesBuffers", indicesHandlers[i], i);
    }

    for (int i = 0; i < images.size(); i++) {
        descriptorSet.Push("ImageSamplers", images[i], i);
    }

    descriptorSet.Push("InstanceDatas", instanceDataHandler);
}
}   // namespace MapleLeaf