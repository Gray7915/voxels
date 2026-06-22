#pragma once
#include "../lve_game_object.hpp"
#include "../lve_device.hpp"
#include "Chunk.hpp"
#include "../lve_util.hpp"

namespace lve
{
    class ChunkRenderer
    {
    public:
        struct Pass
        {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;

            Pass() : vertices(), indices() {};
        };

        static void emit_tile(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices, glm::ivec3 pos, int blocks[16][32][16], glm::vec3 worldOffset);
        static LveGameObject mesh(int blocks[16][32][16], LveDevice &lveDevice, glm::vec3 offset);
        glm::vec2 static getAtlasUV(int face, glm::vec2 uv);
        static glm::ivec3 getDirection(int i)
        {
            return ((glm::ivec3[]){
                glm::ivec3(0, 0, 1),
                glm::ivec3(0, 0, -1),
                glm::ivec3(1, 0, 0),
                glm::ivec3(-1, 0, 0),
                glm::ivec3(0, 1, 0),
                glm::ivec3(0, -1, 0)})[i];
        }
    };
}
