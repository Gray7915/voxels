#pragma once
#include <unordered_map>
<<<<<<< HEAD
=======
#include <memory>
>>>>>>> 6b374db (some stuff)
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "Rendering/Core/lve_device.hpp"
#include "Rendering/Core/lve_model.hpp"
#include "Util/IVec3Hash.h"
#include "World/Chunk.hpp"

namespace lve
{
    class AccelerationStructure
    {
    public:
        AccelerationStructure(LveDevice &device);
        ~AccelerationStructure();

<<<<<<< HEAD
        void buildBLAS(const glm::ivec3 &chunkPos, const LveModel &model);
=======
        void buildBLAS(const glm::ivec3 &chunkPos, const LveModel &model, VkCommandBuffer cmd);
>>>>>>> 6b374db (some stuff)
        void rebuildTLAS(const std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash> &chunks);

        VkAccelerationStructureKHR getTLAS() const { return tlas; }

<<<<<<< HEAD
        bool needsTLASUpdate() const;
        void clearTLASDirty();

    private:
        void primitiveToGeometry(const LveModel &model, VkAccelerationStructureGeometryKHR &geometry, VkAccelerationStructureBuildRangeInfoKHR &rangeInfo);

        VkAccelerationStructureKHR createAccelerationStructure(VkAccelerationStructureTypeKHR type, VkAccelerationStructureGeometryKHR &geometry,
                                                               VkAccelerationStructureBuildRangeInfoKHR &rangeInfo, VkBuildAccelerationStructureFlagsKHR flags,
                                                               VkBuffer &outBuffer, VkDeviceMemory &outMemory);
=======
    private:
        void primitiveToGeometry(
            const LveModel &model,
            VkAccelerationStructureGeometryKHR &geometry,
            VkAccelerationStructureBuildRangeInfoKHR &rangeInfo);

        VkAccelerationStructureKHR createAccelerationStructure(
            VkAccelerationStructureTypeKHR type,
            VkAccelerationStructureGeometryKHR &geometry,
            VkAccelerationStructureBuildRangeInfoKHR &rangeInfo,
            VkBuildAccelerationStructureFlagsKHR flags,
            VkBuffer &outBuffer,
            VkDeviceMemory &outMemory);
>>>>>>> 6b374db (some stuff)

        LveDevice &lveDevice;

        std::unordered_map<glm::ivec3, VkAccelerationStructureKHR, IVec3Hash> blasMap;
        std::unordered_map<glm::ivec3, VkBuffer, IVec3Hash> blasBuffers;
        std::unordered_map<glm::ivec3, VkDeviceMemory, IVec3Hash> blasMemories;

        VkAccelerationStructureKHR tlas = VK_NULL_HANDLE;
        VkBuffer tlasBuffer = VK_NULL_HANDLE;
        VkDeviceMemory tlasMemory = VK_NULL_HANDLE;

        VkPhysicalDeviceAccelerationStructurePropertiesKHR asProperties{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR};
<<<<<<< HEAD

        bool blasDirty = false;
    };
}
=======
    };
}
>>>>>>> 6b374db (some stuff)
