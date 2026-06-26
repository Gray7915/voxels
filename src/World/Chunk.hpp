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
        Chunk(LveDevice &lveDevice, glm::vec3 offset);
        ~Chunk();
        static const int width = 16;
        static const int height = 128;
        inline static const glm::ivec3 CHUNK_SIZE{16, 128, 16};
        glm::vec3 offset;
        enum BlockType : uint8_t
        {
            Air,
            Solid
        };

        int blocks[18][128][18];
        void createChunk(LveDevice &lveDevice, glm::vec3 offset);
        void buildMesh(LveDevice &lveDevice);
        void createTrees();
        std::shared_ptr<LveModel> chunkModel{};
        TransformComponent transform{};

        struct TransformComponent
        {
            glm::vec3 translation{};
            glm::vec3 scale{1.f, 1.f, 1.f};
            glm::vec3 rotation;

            glm::mat4 mat4();
            glm::mat3 normalMatrix();
        };
    };
}
