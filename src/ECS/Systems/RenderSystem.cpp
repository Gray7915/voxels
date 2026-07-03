#include "RenderSystem.hpp"
#include "ECS/Components/Transform.hpp"
#include "ECS/Components/Renderable.hpp"

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
