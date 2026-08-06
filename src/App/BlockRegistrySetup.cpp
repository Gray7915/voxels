#include <glm/glm.hpp>

#include "App/BlockRegistrySetup.hpp"
#include "Physics/BoxVolume.hpp"
#include "Util/Types.hpp"
#include "Util/Direction.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>
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

            block.renderType = parseRenderType(Jblock["renderType"]);

            if (Jblock.contains("behaviour"))
            {
                auto &b = Jblock["behaviour"];

                block.behaviour.behaviourType = parseBehaviourType(b["type"]);
                /*
                             block.behaviour.bitOffset = b["bitOffset"];
                             block.behaviour.bitMask = b["bitMask"];

                             for (auto &field : b["fields"]) {
                                 block.behaviour.fields.push_back({field["name"], field["offset"], field["mask"]});
                             }*/
            }

            if (Jblock.contains("boundingBoxes"))
            {
                for (auto &box : Jblock["boundingBoxes"])
                {
                    BoxVolume volume;

                    volume.boxSize = {box["boxSize"]["x"], box["boxSize"]["y"], box["boxSize"]["z"]};

                    volume.offset = {box["offset"]["x"], box["offset"]["y"], box["offset"]["z"]};

                    block.boundingBoxes.push_back(volume);
                }
            }

            if (Jblock.contains("highlightShape"))
            {
                block.highlightShape.highlightBoxVerticies.clear();
                block.highlightShape.highlighBoxEdges.clear();
                const auto &highlightShape = Jblock["highlightShape"];

                for (const auto &vertex : highlightShape["vertices"])
                {
                    block.highlightShape.highlightBoxVerticies.emplace_back(vertex["x"], vertex["y"], vertex["z"]);
                }

                for (const auto &edge : highlightShape["edges"])
                {
                    block.highlightShape.highlighBoxEdges.push_back({edge[0], edge[1]});
                }
            }

            if (block.renderType == RenderType::Mesh)
            {
                block.model = LveModel::createModelFromFile(device, block.modelName);
            }

            std::cout << " render type" << Jblock["renderType"] << '\n';
            blockRegistry.Register(block);
        }
    }

    BlockBehaviourType BlockRegistrySetup::parseBehaviourType(const std::string &str)
    {
        if (str == "none")
            return BlockBehaviourType::NONE;
        if (str == "staged")
            return BlockBehaviourType::STAGED;
        if (str == "connected")
            return BlockBehaviourType::CONNECTED;
        if (str == "random_offset")
            return BlockBehaviourType::RANDOM_OFFSET;
        if (str == "custom")
            return BlockBehaviourType::CUSTOM;

        // unknown type in json — fail loud so you know immediately
        throw std::runtime_error("Unknown behaviour type: " + str);
    }

    RenderType BlockRegistrySetup::parseRenderType(const std::string &str)
    {
        if (str == "invisible")
            return RenderType::Invisible;
        if (str == "transparent")
            return RenderType::Transparent;
        if (str == "block")
            return RenderType::Block;
        if (str == "mesh")
            return RenderType::Mesh;
    }
} // namespace lve
