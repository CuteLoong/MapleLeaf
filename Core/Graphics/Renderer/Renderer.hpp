#pragma once

#include "GlobalAttachmentsHandler.hpp"
#include "Pipeline.hpp"
#include "RenderStage.hpp"
#include "SubrenderHolder.hpp"
#include "vector"

namespace MapleLeaf {
class Renderer
{
    friend class Graphics;

public:
    Renderer()          = default;
    virtual ~Renderer() = default;

    virtual void Start()  = 0;
    virtual void Update() = 0;

    RenderStage* GetRenderStage(uint32_t index) const
    {
        if (renderStages.empty() || renderStages.size() < index) return nullptr;

        return renderStages.at(index).get();
    }

    void AddRenderStage(std::unique_ptr<RenderStage>&& renderStage) { renderStages.emplace_back(std::move(renderStage)); }

    void CreateGlobalAttachmentsHanlder(const std::vector<FrameAttachment>& frameAttachmentTypes)
    {
        globalAttachmentsHandler = std::make_unique<GlobalAttachmentsHandler>(frameAttachmentTypes);
    }

    GlobalAttachmentsHandler* GetGlobalAttachmentsHandler() const { return globalAttachmentsHandler.get(); }

    template<typename T>
    bool HasSubrender() const
    {
        return subrenderHolder.Has<T>();
    }

    template<typename T>
    T* GetSubrender() const
    {
        return subrenderHolder.Get<T>();
    }

    template<typename T, typename... Args>
    T* AddSubrender(const Pipeline::Stage& pipelineStage, Args&&... args)
    {
        return subrenderHolder.Add<T>(pipelineStage, std::make_unique<T>(pipelineStage, std::forward<Args>(args)...));
    }

    template<typename T>
    void RemoveSubrender()
    {
        subrenderHolder.Remove<T>();
    }

    void ClearSubrenders() { subrenderHolder.Clear(); }

private:
    bool                                      started = false;
    std::vector<std::unique_ptr<RenderStage>> renderStages;
    SubrenderHolder                           subrenderHolder;

    std::unique_ptr<GlobalAttachmentsHandler> globalAttachmentsHandler;
};
}   // namespace MapleLeaf