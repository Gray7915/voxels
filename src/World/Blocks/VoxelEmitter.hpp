#pragma once
#include <string>
#include "Util/Types.hpp"
#include "Util/lve_util.hpp"
#include <vector>
#include "World/voxel.hpp"

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

    struct MeshOutput {
        std::vector<lve::Vertex> &vertices;
        std::vector<u32> &indices;
    };

    class IVoxelContext {
      public:
        virtual ~IVoxelContext() = default;
        virtual Voxel voxelAt(ivec3 p) const = 0;

        virtual bool cullFace(ivec3 pos, int face) const { return false; }
        virtual float ao(ivec3 pos, int face, size_t cubeVertex) const { return 0.9f; }
        virtual vec3 origin(ivec3 pos) const { return vec3(pos); }
        virtual vec3 color() const { return {1, 1, 1}; }
    };

    void emitVoxel(const IVoxelContext &context, MeshOutput &out, ivec3 pos);

    vec2 getAtlasUV(int face, glm::vec2 uv, int blockType);
    vec2 getModelAtlasUV(glm::vec2 modelUV, const std::string &textureName);

} // namespace Blocks