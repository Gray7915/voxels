#pragma once
#include <unordered_map>
#include <memory>
#include <glm/ext/vector_int3.hpp>
#include "../Util/IVec3Hash.h"
#include "../Util/noise.hpp"
#include "../Rendering/Core/lve_device.hpp"
#include "../Rendering/Core/lve_model.hpp"

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

        bool dirty;

        glm::vec3 offset;
        glm::vec3 rotation;
        glm::vec3 scale;

        glm::mat4 mat4();
        glm::mat3 normalMatrix();

        enum BlockType : uint8_t
        {
            Air,
            Solid
        };

        struct Transform
        {
            glm::vec3 position;
            glm::vec3 rotation;
            glm::vec3 scale;

            glm::mat4 mat4();
            glm::mat3 normalMatrix();
        };

        int blocks[18][128][18];
        void createChunk(LveDevice &lveDevice, glm::vec3 offset);
        void buildMesh(LveDevice &lveDevice);
        void createTrees();
        std::shared_ptr<LveModel> chunkModel{};
    };
}
