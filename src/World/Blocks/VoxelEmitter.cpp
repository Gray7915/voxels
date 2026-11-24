#include "World/Blocks/VoxelEmitter.hpp"
#include "World/Blocks/BlockRegistry.hpp"
#include "World/Blocks/ConnectedBlock.hpp"
#include "World/Blocks/StagedBlock.hpp"
#include "World/Blocks/CreateVoxel.hpp"
#include "App/TextureAtlas.hpp"
#include "Util/Direction.hpp"
namespace Blocks
{
    static void emitBlock(const IVoxelContext &context, MeshOutput &out, ivec3 pos, BlockId blockType) {
        const vec3 origin = context.origin(pos);
        for (u8 face = 0; face < static_cast<u8>(Math::Direction::COUNT) - 1; ++face) {
            if (context.cullFace(pos, face)) {
                continue;
            }

            const u32 base = static_cast<u32>(out.vertices.size());
            for (int vert = 0; vert < 4; ++vert) {
                usize cv = CUBE_INDICES[face * 6 + UNIQUE_INDICES[vert]];

                lve::Vertex vertex{};
                vertex.position = origin + CUBE_VERTICES[cv];
                vertex.normal = CUBE_NORMALS[face];
                vertex.uv = getAtlasUV(face, CUBE_UVS[vert], blockType);
                vertex.color = context.color();
                vertex.ao = context.ao(pos, face, cv);
                out.vertices.push_back(vertex);
            }

            for (usize i : FACE_INDICES) {
                out.indices.push_back(base + static_cast<u32>(i));
            }
        }
    }

    static void appendModelSection(const ModelSection &section, vec3 offset, float ao, MeshOutput &out) {
        u32 base = static_cast<u32>(out.vertices.size());
        for (Vertex v : section.vertices) {
            v.position += offset;
            v.color = {1, 1, 1};
            v.ao = ao;
            v.uv.y = 1.0f - v.uv.y;
            v.uv = getModelAtlasUV(v.uv, "fenceTexture");
            out.vertices.push_back(v);
        }
        for (auto i : section.indices)
            out.indices.push_back(base + i);
    }

    static void emitConnectedBlock(const IVoxelContext &ctx, MeshOutput &out, ivec3 pos, const Block &def, Voxel voxel) {
        // Compute the connection mask locally instead of writing back into job.voxelData
        for (ivec3 dir : Math::CardinalDirections) {
            auto rt = BlockRegistry::Get().GetBlockByID(ctx.voxelAt(pos + dir).blockID)->renderType;
            if (rt == RenderType::Block || rt == RenderType::Mesh || rt == RenderType::Transparent)
                ConnectedBlock::setSegmentBit(voxel, Math::VectorToCardinal(dir), true);
        }

        const vec3 offset = ctx.origin(pos) + vec3(0.5f, 0, 0.5f);
        for (auto &sec : def.model->modelSections) {
            if (sec.dirction == Math::Direction::CENTER || ConnectedBlock::isSegmentBitActive(voxel, sec.dirction))
                appendModelSection(sec, offset, 0.9f, out);
        }
    }

    static void emitStagedBlock(const IVoxelContext &ctx, MeshOutput &out, ivec3 pos, const Block &def, const Voxel &voxel) {
        u8 stage = StagedBlock::getBlockStage(voxel);
        const vec3 offset = ctx.origin(pos) + vec3(0.5f, 0, 0.5f);
        for (auto &sec : def.model->modelSections)
            if (sec.stage == stage)
                appendModelSection(sec, offset, 1.0f, out);
    }

    void emitVoxel(const IVoxelContext &ctx, MeshOutput &out, ivec3 pos) {
        Voxel voxel = ctx.voxelAt(pos);
        auto def = BlockRegistry::Get().GetBlockByID(voxel.blockID);
        if (!def)
            return;

        switch (def->behaviour.behaviourType) {
        case BlockBehaviourType::NONE:
            emitBlock(ctx, out, pos, voxel.blockID);
            break;
        case BlockBehaviourType::CONNECTED:
            emitConnectedBlock(ctx, out, pos, *def, voxel);
            break;
        case BlockBehaviourType::STAGED:
            emitStagedBlock(ctx, out, pos, *def, voxel);
            break;
        case BlockBehaviourType::DIRECTIONAL:
            break;
        }
    }

    vec2 getAtlasUV(int face, vec2 uv, int blockType) {
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

    vec2 getModelAtlasUV(vec2 modelUV, const std::string &textureName) {
        const auto &atlas = TextureAtlas::Get();
        const auto it = atlas.atlasRegions.find(textureName);
        const auto &region = it->second;
        return {(region.x + modelUV.x * region.width) / static_cast<float>(atlas.atlasWidth), (region.y + modelUV.y * region.height) / static_cast<float>(atlas.atlasHeight)};
    }

} // namespace Blocks