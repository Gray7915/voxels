#pragma once
#include <memory>
#include "Physics/BoxVolume.hpp"
#include "Rendering/Core/lve_model.hpp"
#include "Util/Direction.hpp"

namespace lve
{
    enum class RenderType : u8
    {
        Invisible,
        Transparent,
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
        std::string modelName;
        bool isSolid = true;
        // AABB collision boxes for each rotation
        std::vector<BoxVolume> boundingBoxes;
        // Render options struct? contains things like is transparent, ect

        // mesh / model -> optional only for non blocks (things like chest count as non-block)
        std::array<std::string, static_cast<size_t>(6)> faces;

        glm::vec3 highlightBoxSize = {1, 1, 1};

        int hardness;

        RenderType renderType = RenderType::Block;

        std::shared_ptr<LveModel> model;
    };
}
