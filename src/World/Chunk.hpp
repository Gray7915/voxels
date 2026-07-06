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

        void uploadMesh(LveDevice &lveDevice, std::vector<lve::Vertex> verticies, std::vector<uint32_t> indices)
        {
            std::cout << "vert count" << verticies.size() << '\n';
            std::cout << "indice count" << indices.size() << '\n';

            auto newModel = LveModel::createChunkModel(lveDevice, verticies, indices);
            if (chunkModel)
            {
                auto oldModel = std::shared_ptr<LveModel>(std::move(chunkModel));
                lveDevice.queueDeletion([model = oldModel]() {}, currentFrameIndex);
            }

            chunkModel = std::move(newModel);
        }
        std::shared_ptr<LveModel> chunkModel{};
    };
}
