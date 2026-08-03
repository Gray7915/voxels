#pragma once
#include <memory>
#include "Physics/BoxVolume.hpp"
#include "Rendering/Core/lve_model.hpp"
#include "Util/Direction.hpp"

namespace lve
{
    enum class RenderType : u8 {
        Invisible,
        Transparent,
        Block,
        Mesh,
    };

    enum class BlockBehaviourType : u8 { NONE = 0, CONNECTED = 1, STAGED = 2, RANDOM_OFFSET = 3, CUSTOM = 4 };

    struct BitField {
        std::string name;
        uint8_t offset;
        uint8_t mask;
    };

    struct BlockBehaviour {
        BlockBehaviourType behaviourType = BlockBehaviourType::NONE;

        uint8_t bitOffset = 0;
        uint8_t bitMask = 0;

        bool connectsToSolid = true;
        uint8_t stageCount = 1;
        float maxRandomOffset = 0.25f;

        std::vector<BitField> fields;
    };

    struct HighlightShape {
        std::vector<ivec2> highlighBoxEdges{};
        std::vector<vec3> highlightBoxVerticies{};
    };

    class Block {
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
        HighlightShape highlightShape{
            .highlighBoxEdges = {{0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}},
            .highlightBoxVerticies = {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}}};

        int hardness;

        RenderType renderType = RenderType::Block;

        BlockBehaviour behaviour{.behaviourType = BlockBehaviourType::NONE};

        std::shared_ptr<LveModel> model;
    };
} // namespace lve
