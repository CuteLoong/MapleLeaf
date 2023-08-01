#pragma once

#include "Component.hpp"
#include "DescriptorHandler.hpp"
#include "PipelineGraphics.hpp"
#include "PushHandler.hpp"


namespace MapleLeaf {
class ShadowRender : public Component::Registrar<ShadowRender>
{
    inline static const bool Registrar = Register("shadowRender");

public:
    ShadowRender();

    void Start() override;
    void Update() override;

    bool CmdRender(const CommandBuffer& commandBuffer, const PipelineGraphics& pipeline);

private:
    DescriptorsHandler descriptorSet;
    PushHandler        pushObject;
};
}   // namespace MapleLeaf