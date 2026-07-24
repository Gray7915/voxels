#include <glm/glm.hpp>

#include "App/BlockRegistrySetup.hpp"
#include "Physics/BoxVolume.hpp"
namespace lve
{
    void BlockRegistrySetup::SetupBlockRegistry(BlockRegistry &blockRegistry, LveDevice &device)
    {
        blockRegistry.Register({.id = uint16_t(0), .name = "air", .isSolid = false});
        blockRegistry.Register({.id = uint16_t(1), .name = "grass-top", .isSolid = true});
        blockRegistry.Register({.id = uint16_t(2), .name = "dirt", .isSolid = true});
        blockRegistry.Register({.id = uint16_t(3), .name = "stone", .isSolid = true});

        blockRegistry.Register({.id = uint16_t(4), .name = "fenceTexture", .modelName = "../models/fence.obj", .isSolid = true, .boundingBoxes = {BoxVolume{.boxSize = glm::vec3(0.25, 1, 0.25), .offset{0.5, 0, 0.5}}}, .highlightBoxSize = {0.25, 1, 0.25}, .hardness = 0, .renderType = RenderType::Mesh, .model = LveModel::createModelFromFile(device, "models/fence.obj")});
    }
}
