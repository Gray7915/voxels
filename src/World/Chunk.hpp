#pragma once
#include <unordered_map>
#include <memory>
#include <iostream>
#include <glm/ext/vector_int3.hpp>
#include "../Util/IVec3Hash.h"
#include "../Util/noise.hpp"
#include "../Util/ChunkState.hpp"
#include "../Rendering/Core/lve_device.hpp"
#include "../Rendering/Core/lve_model.hpp"
#include "World/VoxelData.hpp"

namespace lve
{
    class Chunk
    {
    public:
        Chunk(LveDevice &lveDevice, glm::vec3 offset);
        ~Chunk();
        static const int width = 16;
        static const int height = 128;
        inline static const glm::ivec3 CHUNK_SIZE{16, 128, 16};

        VoxelData voxelData;

        ChunkState chunkState = ChunkState::Unloaded;

        glm::vec3 offset;
        glm::vec3 rotation{0.0f};
        glm::vec3 scale{1.0f};

        glm::mat4 mat4();
        glm::mat3 normalMatrix();

        enum BlockType : uint8_t
        {
            Air,
            Solid
        };

        struct Transform
        {
            glm::mat4 mat4();
            glm::mat3 normalMatrix();
        };

        void setVoxelData(VoxelData data)
        {
            this->voxelData = data;
        };

        void applyMesh(std::unique_ptr<LveModel> model, int currentFrameIndex, LveDevice &device)
        {
            if (chunkModel)
            {
                auto oldModel = std::shared_ptr<LveModel>(std::move(chunkModel));
                device.queueDeletion([model = oldModel]() {}, currentFrameIndex);
            }

            chunkModel = std::move(model);
        }
        std::shared_ptr<LveModel> chunkModel{};
    };
}
