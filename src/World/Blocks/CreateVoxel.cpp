#include "World/Blocks/CreateVoxel.hpp"
#include "World/Blocks/BlockRegistry.hpp"
#include "World/Blocks/ConnectedBlock.hpp"
#include "World/Blocks/StagedBlock.hpp"
#include "Rendering/Core/lve_device.hpp"
#include "App/TextureAtlas.hpp"
#include "Util/Direction.hpp"
#include "Util/Types.hpp"
#include "Util/Constants.hpp"
namespace Blocks
{
    static const size_t UNIQUE_INDICES[] = {0, 1, 2, 5};
    static const size_t FACE_INDICES[] = {0, 1, 2, 0, 2, 3};

    static const size_t CUBE_INDICES[] = {
        4, 7, 6, 4, 6, 5, // south (+z)
        3, 0, 1, 3, 1, 2, // north (-z)
        7, 3, 2, 7, 2, 6, // east  (+x)
        0, 4, 5, 0, 5, 1, // west  (-x)
        2, 1, 5, 2, 5, 6, // up    (+y)
        0, 3, 7, 0, 7, 4  // down  (-y)
    };

    static const vec3 CUBE_VERTICES[] = {{0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 1}, {0, 1, 1}, {1, 1, 1}, {1, 0, 1}};

    static const vec3 CUBE_NORMALS[] = {{0, 0, 1}, {0, 0, -1}, {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}};

    static const vec2 CUBE_UVS[] = {{0, 1}, {1, 1}, {1, 0}, {0, 0}};

    void CreateVoxel::emitVoxel(MeshJob &job, MeshResult &result, ivec3 pos) {
        // std::cout << "voxel emitted" << '\n';
        int BlockID = job.voxelData.get(pos.x, pos.y, pos.z);
        auto blockDef = BlockRegistry::Get().GetBlockByID(BlockID);
        switch (blockDef->behaviour.behaviourType) {
        case BlockBehaviourType::NONE:
            emitBlock(job, result, pos);
            break;
        case BlockBehaviourType::CONNECTED:
            emitConnectedBlock(job, result, pos);
            break;
        case BlockBehaviourType::STAGED:
            // emitStagedBlock(job, result, pos);
            break;
        case BlockBehaviourType::DIRECTIONAL:
            // emitDirectionalBlock(job, result, pos);
            break;
        }
    }

    void CreateVoxel::emitCubeFaces(std::vector<Vertex> &vertices, std::vector<u32> &indices, const CubeFaceParams &p) {
        for (u8 face = 0; face < static_cast<u8>(Math::Direction::COUNT) - 1; ++face) {
            if (p.shouldEmitFace && !p.shouldEmitFace(face))
                continue;

            const uint32_t baseIndex = static_cast<uint32_t>(vertices.size());

            for (int vert = 0; vert < 4; ++vert) {
                size_t cubeVertex = CUBE_INDICES[face * 6 + UNIQUE_INDICES[vert]];

                Vertex v{};
                v.position = p.origin + CUBE_VERTICES[cubeVertex];
                v.normal = CUBE_NORMALS[face];
                v.uv = getAtlasUV(face, CUBE_UVS[vert], p.blockType);
                v.color = p.color;
                v.ao = p.aoForVertex ? p.aoForVertex(face, cubeVertex) : p.defaultAO;
                vertices.push_back(v);
            }

            for (size_t i : FACE_INDICES)
                indices.push_back(baseIndex + static_cast<uint32_t>(i));
        }
    }

    void CreateVoxel::emitBlock(MeshJob &job, MeshResult &result, ivec3 pos) {
        int blockType = job.voxelData.get(pos.x, pos.y, pos.z);

        for (u8 face = 0; face < static_cast<u8>(Math::Direction::COUNT) - 1; ++face) {
            ivec3 n = pos + Math::DirectionByFaceInt(face);

            bool neighborSolid = BlockRegistry::Get().GetBlockByID(getChunkVoxel(job, n).blockID)->renderType == RenderType::Block;
            if (neighborSolid) {
                continue;
            }

            const uint32_t baseIndex = static_cast<uint32_t>(result.verticies.size());
            for (int vert = 0; vert < 4; vert++) {
                size_t cubeVertex = CUBE_INDICES[face * 6 + UNIQUE_INDICES[vert]];

                Vertex vertex{};
                vertex.position = glm::vec3(pos) + CUBE_VERTICES[cubeVertex];
                vertex.normal = CUBE_NORMALS[face];
                vertex.uv = getAtlasUV(face, CUBE_UVS[vert], blockType);
                vertex.color = {1, 1, 1};
                int ao = calculateAO(pos, face, cubeVertex, job);

                vertex.ao = aoValues[ao];
                result.verticies.push_back(vertex);
            }

            for (size_t i : FACE_INDICES)
                result.indices.push_back(baseIndex + static_cast<uint32_t>(i));
        }
    }

    void CreateVoxel::emitConnectedBlock(MeshJob &job, MeshResult &result, ivec3 pos) {
        auto block = BlockRegistry::Get().GetBlockByID(getChunkVoxel(job, pos).blockID);
        if (!block)
            return;

        Voxel voxel = job.voxelData.getVoxel(pos.x, pos.y, pos.z);
        ConnectedBlock::setSegmentBit(voxel, Math::VectorToCardinal({0, 0, 0}), true);
        for (ivec3 dir : Math::CardinalDirections) {
            ivec3 n = pos + dir;
            RenderType renderType = BlockRegistry::Get().GetBlockByID(getChunkVoxel(job, n).blockID)->renderType;
            bool neighborSolid = renderType == RenderType::Block || renderType == RenderType::Mesh || renderType == RenderType::Transparent;
            if (neighborSolid) {
                ConnectedBlock::setSegmentBit(voxel, Math::VectorToCardinal(dir), true);
                job.voxelData.setVoxelData(pos.x, pos.y, pos.z, voxel);
            }

            for (auto &modelSection : block->model->modelSections) {
                if (ConnectedBlock::isSegmentBitActive(job.voxelData.getVoxel(pos.x, pos.y, pos.z), modelSection.dirction) || modelSection.dirction == Math::Direction::CENTER) {
                    u32 baseVertex = static_cast<u32>(result.verticies.size());
                    for (Vertex vert : modelSection.vertices) {
                        vert.position = vert.position + vec3(pos) + vec3(0.5, 0, 0.5);
                        vert.color = {1.f, 1.f, 1.f};
                        vert.ao = 0.9f;
                        vert.uv.y = 1.0f - vert.uv.y;
                        vert.uv = getModelAtlasUV(vert.uv, "fenceTexture");
                        result.verticies.push_back(vert);
                    }

                    for (auto index : modelSection.indices) {
                        result.indices.push_back(baseVertex + index);
                    }
                }
            }
        }
    }

    void CreateVoxel::emitStagedBlock(MeshJob &job, MeshResult &result, ivec3 pos) {
        auto block = BlockRegistry::Get().GetBlockByID(getChunkVoxel(job, pos).blockID);
        u8 stage = StagedBlock::getBlockStage(getChunkVoxel(job, pos));
        if (!block)
            return;

        u32 baseVertex = static_cast<u32>(result.verticies.size());
        for (auto &modelSection : block->model->modelSections) {
            if (modelSection.stage != stage)
                continue;

            for (Vertex vert : modelSection.vertices) {
                vert.position = vert.position + vec3(pos) + vec3(0.5, 0, 0.5);
                vert.color = {1.f, 1.f, 1.f};
                vert.ao = 1.f;
                vert.uv.y = 1.0f - vert.uv.y;
                vert.uv = getModelAtlasUV(vert.uv, "fenceTexture");
                result.verticies.push_back(vert);
            }

            for (auto index : modelSection.indices) {
                result.indices.push_back(baseVertex + index);
            }
        }
    }

    int CreateVoxel::getSign(ivec3 tangent, ivec3 vertex) {
        if (tangent.x != 0)
            return vertex.x ? 1 : -1;

        if (tangent.y != 0)
            return vertex.y ? 1 : -1;

        return vertex.z ? 1 : -1;
    }

    glm::ivec3 CreateVoxel::getFaceTangent1(int face) {
        if (face <= 1 || face >= 4)
            return {1, 0, 0};
        if (face >= 2 && face <= 3)
            return {0, 0, 1};
        return {};
    }

    glm::ivec3 CreateVoxel::getFaceTangent2(int face) {
        if (face <= 3)
            return {0, 1, 0};
        if (face <= 5)
            return {0, 0, 1};
        return {};
    }

    int CreateVoxel::calculateAO(ivec3 pos, int face, int cubeVertex, MeshJob &job) {
        ivec3 localVertex = ivec3(CUBE_VERTICES[cubeVertex]);

        ivec3 tangent1 = getFaceTangent1(face);
        ivec3 tangent2 = getFaceTangent2(face);

        int sign1 = getSign(tangent1, localVertex);
        int sign2 = getSign(tangent2, localVertex);

        ivec3 side1 = tangent1 * sign1;
        ivec3 side2 = tangent2 * sign2;
        ivec3 corner = side1 + side2;

        ivec3 faceDir = Math::DirectionByFaceInt(face);

        int block1 = getSolid(pos + faceDir + side1, job);
        int block2 = getSolid(pos + faceDir + side2, job);
        int blockCorner = getSolid(pos + faceDir + corner, job);

        if (block1 + block2 == 2)
            return 0;

        return 3 - (block1 + block2 + blockCorner);
    }

    int CreateVoxel::getSolid(ivec3 voxel, const MeshJob &job) {
        if (voxel.y < 0 || voxel.y >= VoxelData::HEIGHT)
            return 0;
        return (BlockRegistry::Get().GetBlockByID(getChunkVoxel(job, voxel).blockID)->renderType == RenderType::Block) ? 1 : 0;
    }

    vec2 CreateVoxel::getAtlasUV(int face, vec2 uv, int blockType) {
        /*
        For models need to change this
        Probably have the texture put into the atlas at us the regular model uvs combined with
        the texture placement on the atlas to correctly texture the model
        atlasUV = regionOffset + (modelUV * regionSize)?
        */
        auto block = BlockRegistry::Get().GetBlockByID(blockType);

        if (!block) {
            std::cerr << "Missing block ID: " << blockType << '\n';
            return {0, 0};
        }

        std::string requstTexture = BlockRegistry::Get().GetBlockByID(blockType)->faces.at(face);
        // std::cout << requstTexture << '\n';

        const auto &region = TextureAtlas::Get().atlasRegions.find(requstTexture);
        if (region != TextureAtlas::Get().atlasRegions.end()) {
            const auto &atlas = TextureAtlas::Get();

            float u = (region->second.x + uv.x * region->second.width) / static_cast<float>(atlas.atlasWidth);

            float v = (region->second.y + uv.y * region->second.height) / static_cast<float>(atlas.atlasHeight);

            return {u, v};
        }
        return uv;
    }

    vec2 CreateVoxel::getModelAtlasUV(vec2 modelUV, const std::string &textureName) {
        const auto &atlas = TextureAtlas::Get();
        const auto it = atlas.atlasRegions.find(textureName);
        const auto &region = it->second;
        return {(region.x + modelUV.x * region.width) / static_cast<float>(atlas.atlasWidth), (region.y + modelUV.y * region.height) / static_cast<float>(atlas.atlasHeight)};
    }

    Voxel CreateVoxel::getChunkVoxel(const MeshJob &job, glm::ivec3 v) {
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
} // namespace Blocks