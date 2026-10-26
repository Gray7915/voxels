#pragma once

#include <vector>
#include "Util/Types.hpp"
#include "World/VoxelData.hpp"
#include "Util/ThreadSafeQueue.hpp"

namespace Blocks
{
    using namespace lve;

    class CreateVoxel {

        struct CubeFaceParams {
            BlockId blockType;
            vec3 origin;
            vec3 color{1, 1, 1};

            std::function<bool(int face)> shouldEmitFace = nullptr;
            std::function<float(int face, size_t cubeVertex)> aoForVertex = nullptr;
            float defaultAO = 0.9f;
        };

      public:
        void static emitVoxel(MeshJob &job, MeshResult &result, ivec3 pos);

      private:
        static constexpr float aoValues[] = {0.1f, 0.34f, 0.65f, 1.f};

        static void emitCubeFaces(std::vector<Vertex> &vertices, std::vector<u32> &indices, const CubeFaceParams &p);

        static void emitBlock(MeshJob &job, MeshResult &result, ivec3 pos);
        std::unique_ptr<LveModel> emitBlock(BlockId blockId);

        static void emitConnectedBlock(MeshJob &job, MeshResult &result, ivec3 pos);
        std::unique_ptr<LveModel> emitConnectedBlock(BlockId blockId);

        static void emitStagedBlock(MeshJob &job, MeshResult &result, ivec3 pos);
        std::unique_ptr<LveModel> emitStagedBlock(BlockId blockId);

        static void emitDirectionalBlock(MeshJob &job, MeshResult &result, ivec3 pos);
        std::unique_ptr<LveModel> emitDirectionalBlock(BlockId blockId);

        static int calculateAO(glm::ivec3 pos, int face, int vertexIndex, MeshJob &job);
        static ivec3 getDirection(int i);
        static vec2 getAtlasUV(int face, glm::vec2 uv, int blockType);
        static vec2 getModelAtlasUV(glm::vec2 modelUV, const std::string &textureName);

        static ivec3 getFaceTangent1(int face);
        static ivec3 getFaceTangent2(int face);

        static int getSign(ivec3 tangent, ivec3 vertex);
        static int getSolid(ivec3 voxel, const MeshJob &job);

        static Voxel getChunkVoxel(const MeshJob &job, ivec3 chunkVoxel);
    };
} // namespace Blocks