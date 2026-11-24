#include <iostream>
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
            std::cout << "transform comp for rend entity " << transform.position.x << " " << transform.position.y << " " << transform.position.z << " entity " << entity << '\n';
            auto &renderable = coordinator.GetComponent<RenderableComponent>(entity);
            std::cout << renderable.model->modelVerticies.size() << " num verticies" << '\n';

            simpleRenderSystem.renderGameObjects(frameInfo, transform.mat4(), transform.normalMatrix(), renderable.model);
        }
    }
}
