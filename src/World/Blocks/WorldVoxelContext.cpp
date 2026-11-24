#include "World/Blocks/WorldVoxelContext.hpp"
#include "Util/Constants.hpp"
#include "World/VoxelData.hpp"
#include "World/Blocks/BlockRegistry.hpp"

namespace Blocks
{
    Voxel WorldVoxelContext::voxelAt(ivec3 v) const {
        if (v.y < 0 || v.y >= CHUNK_HEIGHT)
            return Voxel{};

        if (v.x >= 0 && v.x < CHUNK_WIDTH && v.y >= 0 && v.y < CHUNK_HEIGHT && v.z >= 0 && v.z < CHUNK_DEPTH) {
            // std::cout << "get voxel " << v.y << " " << v.y << " " << v.z << '\n';
            return job.voxelData.getVoxel(v.x, v.y, v.z);
        }

        // corners — check before edges
        if (v.x == 16 && v.z == 16)
            return job.neighborVoxelData.getVoxel(16, v.y, 0);
        if (v.x == 16 && v.z == -1)
            return job.neighborVoxelData.getVoxel(16, v.y, 3);
        if (v.x == -1 && v.z == 16)
            return job.neighborVoxelData.getVoxel(16, v.y, 1);
        if (v.x == -1 && v.z == -1)
            return job.neighborVoxelData.getVoxel(16, v.y, 2);

        // edges
        if (v.x == 16)
            return job.neighborVoxelData.getVoxel(v.z, v.y, 0);
        if (v.z == 16)
            return job.neighborVoxelData.getVoxel(v.x, v.y, 1);
        if (v.x == -1)
            return job.neighborVoxelData.getVoxel(v.z, v.y, 2);
        if (v.z == -1)
            return job.neighborVoxelData.getVoxel(v.x, v.y, 3);

        return job.neighborVoxelData.getVoxel(0, 0, 0);
    }

    bool WorldVoxelContext::cullFace(ivec3 pos, int face) const {
        ivec3 n = pos + Math::DirectionByFaceInt(face);
        auto def = lve::BlockRegistry::Get().GetBlockByID(voxelAt(n).blockID);
        return def->renderType == lve::RenderType::Block;
    }

    float WorldVoxelContext::ao(ivec3 pos, int face, size_t cubeVertex) const { return aoValues[calculateAO(pos, face, cubeVertex)]; }

    int WorldVoxelContext::getSign(ivec3 tangent, ivec3 vertex) {
        if (tangent.x != 0)
            return vertex.x ? 1 : -1;

        if (tangent.y != 0)
            return vertex.y ? 1 : -1;

        return vertex.z ? 1 : -1;
    }

    ivec3 WorldVoxelContext::faceTangent1(int face) {
        if (face <= 1 || face >= 4)
            return {1, 0, 0};
        if (face >= 2 && face <= 3)
            return {0, 0, 1};
        return {};
    }

    ivec3 WorldVoxelContext::faceTangent2(int face) {
        if (face <= 3)
            return {0, 1, 0};
        if (face <= 5)
            return {0, 0, 1};
        return {};
    }

    int WorldVoxelContext::calculateAO(ivec3 pos, int face, usize cubeVertex) const {
        ivec3 localVertex = ivec3(CUBE_VERTICES[cubeVertex]);

        ivec3 tangent1 = faceTangent1(face);
        ivec3 tangent2 = faceTangent2(face);

        int sign1 = getSign(tangent1, localVertex);
        int sign2 = getSign(tangent2, localVertex);

        ivec3 side1 = tangent1 * sign1;
        ivec3 side2 = tangent2 * sign2;
        ivec3 corner = side1 + side2;

        ivec3 faceDir = Math::DirectionByFaceInt(face);

        int block1 = isSolid(pos + faceDir + side1);
        int block2 = isSolid(pos + faceDir + side2);
        int blockCorner = isSolid(pos + faceDir + corner);

        if (block1 + block2 == 2)
            return 0;

        return 3 - (block1 + block2 + blockCorner);
    }

    int WorldVoxelContext::isSolid(ivec3 p) const {
        if (p.y < 0 || p.y >= lve::VoxelData::HEIGHT)
            return 0;
        return lve::BlockRegistry::Get().GetBlockByID(voxelAt(p).blockID)->renderType == lve::RenderType::Block ? 1 : 0;
    }
} // namespace Blocks