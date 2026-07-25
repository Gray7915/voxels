#include <glm/glm.hpp>

#include "App/BlockRegistrySetup.hpp"
#include "Physics/BoxVolume.hpp"
#include "Util/Types.hpp"
#include "Util/Direction.hpp"

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace lve
{
    void BlockRegistrySetup::SetupBlockRegistry(BlockRegistry &blockRegistry, LveDevice &device)
    {
        std::ifstream file("../Content/blocks.json");
        if (!file.is_open())
        {
            std::cerr << "Failed to open blocks.json\n";
            return;
        }
        json blocks;
        file >> blocks;
        for (const auto &Jblock : blocks)
        {
            Block block;

            block.id = Jblock["id"];
            block.name = Jblock["name"];
            block.isSolid = Jblock["isSolid"];
            block.modelName = Jblock["model"];
            block.hardness = Jblock["hardness"];

            auto &h = Jblock["highlightBoxSize"];
            block.highlightBoxSize = {h["x"], h["y"], h["z"]};

            for (auto &[face, texture] : Jblock["textures"].items())
            {
                if (face == "all")
                {
                    std::string tex = texture;

                    block.faces[(size_t)Math::Direction::UP] = tex;
                    block.faces[(size_t)Math::Direction::DOWN] = tex;
                    block.faces[(size_t)Math::Direction::NORTH] = tex;
                    block.faces[(size_t)Math::Direction::SOUTH] = tex;
                    block.faces[(size_t)Math::Direction::EAST] = tex;
                    block.faces[(size_t)Math::Direction::WEST] = tex;
                }

                if (face == "side")
                {
                    std::string tex = texture;

                    block.faces[(size_t)Math::Direction::NORTH] = tex;
                    block.faces[(size_t)Math::Direction::SOUTH] = tex;
                    block.faces[(size_t)Math::Direction::EAST] = tex;
                    block.faces[(size_t)Math::Direction::WEST] = tex;
                }

                if (face == "north")
                {
                    std::string tex = texture;
                    block.faces[(size_t)Math::Direction::NORTH] = tex;
                }

                if (face == "up")
                {
                    std::string tex = texture;
                    block.faces[(size_t)Math::Direction::UP] = tex;
                }

                if (face == "down")
                {
                    std::string tex = texture;
                    block.faces[(size_t)Math::Direction::DOWN] = tex;
                }
            }

            blockRegistry.Register(block);
        }
        blockRegistry.Register({.id = uint16_t(4), .name = "oak_planks", .modelName = "../models/fence.obj", .isSolid = true, .boundingBoxes = {BoxVolume{.boxSize = glm::vec3(0.25, 1, 0.25), .offset{0.5, 0, 0.5}}}, .highlightBoxSize = {0.25, 1, 0.25}, .hardness = 0, .renderType = RenderType::Mesh, .model = LveModel::createModelFromFile(device, "models/fence.obj")});
    }
}
