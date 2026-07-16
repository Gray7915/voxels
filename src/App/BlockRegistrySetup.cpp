#include <glm/glm.hpp>

#include "App/BlockRegistrySetup.hpp"
#include "Physics/BoxVolume.hpp"
namespace lve
{
    void BlockRegistrySetup::SetupBlockRegistry(BlockRegistry &blockRegistry)
    {
        blockRegistry.Register({.id = uint16_t(0), .name = "Air"});
        blockRegistry.Register({.id = uint16_t(0), .name = "Grass"});
        blockRegistry.Register({.id = uint16_t(0), .name = "Dirt"});
        blockRegistry.Register({.id = uint16_t(0), .name = "Stone"});

        blockRegistry.Register({.id = uint16_t(0), .name = "Vase", .boundingBoxes = {BoxVolume{.boxSize = glm::vec3(0.25, 1, 0.25)}}, .hardness = 0, .renderType = RenderType::Mesh});
    }
}
