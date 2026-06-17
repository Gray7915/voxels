#pragma once
#include <unordered_map>
#include <glm/ext/vector_int3.hpp>
#include "../IVec3Hash.h"
#include "../lve_game_object.hpp"
#include "../Util/noise.hpp"

namespace lve
{
    class Chunk
    {
        lve::Octave noise;

    public:
        Chunk(std::unordered_map<glm::ivec3, LveGameObject, IVec3Hash> &gameObjects, LveDevice &lveDevice, glm::vec3 offset);
        ~Chunk();
        static const int width = 16;
        static const int height = 32;
        glm::vec3 offset;
        enum BlockType : uint8_t
        {
            Air,
            Solid
        };

        int blocks[16][32][16];
        void createChunk(std::unordered_map<glm::ivec3, LveGameObject, IVec3Hash> &gameObjects, LveDevice &lveDevice, glm::vec3 offset);
        void buildMesh(std::unordered_map<glm::ivec3, LveGameObject, IVec3Hash> &gameObjects, LveDevice &lveDevice);
        std::shared_ptr<LveModel> model{};
    };
}
