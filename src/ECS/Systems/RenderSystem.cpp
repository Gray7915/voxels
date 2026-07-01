#include "RenderSystem.hpp"
#include "../Components/Transform.hpp"
#include "../Components/Renderable.hpp"

namespace lve
{
    extern Coordinator coordinator;

    void RenderSystem::Update(FrameInfo &frameInfo, SimpleRenderSystem &simpleRenderSystem)
    {

        for (auto const &entity : mEntities)
        {
            auto &transform = coordinator.GetComponent<Transform>(entity);
            auto &renderable = coordinator.GetComponent<RenderableComponent>(entity);

            simpleRenderSystem.renderGameObjects(frameInfo, transform.mat4(), transform.normalMatrix(), renderable.model);
        }
    }
}
