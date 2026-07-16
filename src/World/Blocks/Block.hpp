#pragma once
#include <memory>
#include "Physics/BoxVolume.hpp"
#include "Rendering/Core/lve_model.hpp"

namespace lve
{
    enum class RenderType
    {
        Block,
        Mesh,
    };

    class Block
    {
    public:
        // Block(uint16_t id, std::string name) : id{id}, name{name} {}
        // Block(uint16_t id, std::string name, std::vector<BoxVolume> boundingBoxes, LveModel mesh);

        uint16_t id;

        std::string name;

        // AABB collision boxes for each rotation
        std::vector<BoxVolume> boundingBoxes;
        // Render options struct? contains things like is transparent, ect

        // mesh / model -> optional only for non blocks (things like chest count as non-block)

        glm::vec3 highlightBoxSize = {1, 1, 1};

        int hardness;

        RenderType renderType = RenderType::Block;
    };
}
