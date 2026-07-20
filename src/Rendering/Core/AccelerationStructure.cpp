#include "AccelerationStructure.hpp"
#include "Rendering/Core/lve_buffer.hpp"
#include <glm/gtc/type_ptr.hpp>
namespace lve
{
    AccelerationStructure::AccelerationStructure(LveDevice &device) : lveDevice{device}
    {
        VkPhysicalDeviceProperties2 props2{};
        props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        props2.pNext = &asProperties;
        vkGetPhysicalDeviceProperties2(lveDevice.hardwareDevice(), &props2);
    }

    AccelerationStructure::~AccelerationStructure()
    {
        // destroy TLAS
        if (tlas != VK_NULL_HANDLE)
            lveDevice.vkDestroyAccelerationStructureKHR(lveDevice.device(), tlas, nullptr);
        if (tlasBuffer != VK_NULL_HANDLE)
            vkDestroyBuffer(lveDevice.device(), tlasBuffer, nullptr);
        if (tlasMemory != VK_NULL_HANDLE)
            vkFreeMemory(lveDevice.device(), tlasMemory, nullptr);

        // destroy all BLASes
        for (auto &[pos, blas] : blasMap)
            lveDevice.vkDestroyAccelerationStructureKHR(lveDevice.device(), blas, nullptr);
        for (auto &[pos, buf] : blasBuffers)
            vkDestroyBuffer(lveDevice.device(), buf, nullptr);
        for (auto &[pos, mem] : blasMemories)
            vkFreeMemory(lveDevice.device(), mem, nullptr);
    }

    void AccelerationStructure::primitiveToGeometry(const LveModel &model, VkAccelerationStructureGeometryKHR &geometry, VkAccelerationStructureBuildRangeInfoKHR &rangeInfo)
    {
        uint64_t vertexAddress = lveDevice.getBufferDeviceAddress(model.getVertexBuffer());
        uint64_t indexAddress = lveDevice.getBufferDeviceAddress(model.getIndexBuffer());

        VkAccelerationStructureGeometryTrianglesDataKHR triangles{};
        triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
        triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        triangles.vertexData = {.deviceAddress = vertexAddress};
        triangles.vertexStride = sizeof(Vertex);
        triangles.maxVertex = model.getVertexCount() - 1;
        triangles.indexType = VK_INDEX_TYPE_UINT32;
        triangles.indexData = {.deviceAddress = indexAddress};

        geometry = {};
        geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        geometry.geometry = {.triangles = triangles};
        geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

        rangeInfo = {};
        rangeInfo.primitiveCount = model.getIndexCount() / 3;
    }

    // Generic function to create an acceleration structure (BLAS or TLAS)
    // Note: This function creates and destroys a scratch buffer for each call.
    // Not optimal but easier to read and understand. See Helper function for a better approach.
    VkAccelerationStructureKHR AccelerationStructure::createAccelerationStructure(
        VkAccelerationStructureTypeKHR type,
        VkAccelerationStructureGeometryKHR &geometry,
        VkAccelerationStructureBuildRangeInfoKHR &rangeInfo,
        VkBuildAccelerationStructureFlagsKHR flags,
        VkBuffer &outBuffer,
        VkDeviceMemory &outMemory)
    {
        auto alignUp = [](VkDeviceSize value, VkDeviceSize alignment) noexcept
        {
            return (value + alignment - 1) & ~(alignment - 1);
        };

        std::cout << "got into method " << '\n';
        // 1. Query build sizes
        VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
        buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        buildInfo.type = type;
        buildInfo.flags = flags;
        buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        buildInfo.geometryCount = 1;
        buildInfo.pGeometries = &geometry;
        std::cout << "did build info " << '\n';
        std::cout << "type: " << buildInfo.type << "\n";
        std::cout << "flags: " << buildInfo.flags << "\n";
        std::cout << "mode: " << buildInfo.mode << "\n";
        std::cout << "srcAS: " << buildInfo.srcAccelerationStructure << "\n";
        std::cout << "dstAS: " << buildInfo.dstAccelerationStructure << "\n";
        uint32_t primCount = rangeInfo.primitiveCount;
        VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
        sizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
        std::cout << "buildInfo.sType = " << buildInfo.sType << "\n";
        std::cout << "geometryCount = " << buildInfo.geometryCount << "\n";
        std::cout << "pGeometries = " << buildInfo.pGeometries << "\n";
        std::cout << "---- BLAS geometry ----\n";
        std::cout << "vertex address: "
                  << geometry.geometry.triangles.vertexData.deviceAddress
                  << "\n";

        std::cout << "index address: "
                  << geometry.geometry.triangles.indexData.deviceAddress
                  << "\n";

        std::cout << "vertex count: "
                  << (geometry.geometry.triangles.maxVertex + 1)
                  << "\n";

        std::cout << "primitive count: "
                  << primCount
                  << "\n";

        std::cout << "vertex stride: "
                  << geometry.geometry.triangles.vertexStride
                  << "\n";
        if (lveDevice.vkGetAccelerationStructureBuildSizesKHR == nullptr)
        {
            throw std::runtime_error("vkGetAccelerationStructureBuildSizesKHR is NULL");
        }

        auto &tri = geometry.geometry.triangles;
        lveDevice.vkGetAccelerationStructureBuildSizesKHR(
            lveDevice.device(),
            VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
            &buildInfo,
            &primCount,
            &sizeInfo);

        std::cout << "did get accel struct build sizes" << '\n';
        VkResult result;

        // after vkGetAccelerationStructureBuildSizesKHR
        std::cout << "AS sizes:"
                  << "\n accel size: " << sizeInfo.accelerationStructureSize
                  << "\n scratch size: " << sizeInfo.buildScratchSize
                  << "\n";
        // 2. Create the AS buffer
        VkDeviceSize asSize = sizeInfo.accelerationStructureSize;
        std::cout << "got size " << '\n';
        lveDevice.createBuffer(asSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, outBuffer, outMemory);

        // 3. Create the acceleration structure
        VkAccelerationStructureCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        createInfo.buffer = outBuffer;
        createInfo.size = asSize;
        createInfo.type = type;

        VkAccelerationStructureKHR accelStruct = VK_NULL_HANDLE;
        result = lveDevice.vkCreateAccelerationStructureKHR(lveDevice.device(), &createInfo, nullptr, &accelStruct);
        if (result != VK_SUCCESS)
        {
            std::cout << "vkCreateAccelerationStructureKHR failed "
                      << result << "\n";
            throw std::runtime_error("failed creating AS");
        }

        // 4. Create scratch buffer
        VkDeviceSize scratchSize = alignUp(
            sizeInfo.buildScratchSize,
            asProperties.minAccelerationStructureScratchOffsetAlignment);

        VkBuffer scratchBuffer;
        VkDeviceMemory scratchMemory;
        lveDevice.createBuffer(
            scratchSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            scratchBuffer,
            scratchMemory);

        uint64_t scratchAddress = lveDevice.getBufferDeviceAddress(scratchBuffer);

        // 5. Build
        buildInfo.dstAccelerationStructure = accelStruct;
        buildInfo.scratchData.deviceAddress = scratchAddress;

        VkCommandBuffer cmd = lveDevice.beginSingleTimeCommands();
        VkAccelerationStructureBuildRangeInfoKHR *pRangeInfo = &rangeInfo;
        lveDevice.vkCmdBuildAccelerationStructures(cmd, 1, &buildInfo, &pRangeInfo);
        lveDevice.endSingleTimeCommands(cmd);

        // 6. Cleanup scratch
        vkDestroyBuffer(lveDevice.device(), scratchBuffer, nullptr);
        vkFreeMemory(lveDevice.device(), scratchMemory, nullptr);
            std::cout << "return accel structu" << '\n';
        return accelStruct;
    }

    void AccelerationStructure::buildBLAS(const glm::ivec3 &chunkPos, const LveModel &model)
    {
        std::cout << "building blas" << '\n';
        // Destroy existing BLAS for this chunk if it exists
        if (blasMap.count(chunkPos))
        {
            lveDevice.vkDestroyAccelerationStructureKHR(lveDevice.device(), blasMap[chunkPos], nullptr);
            vkDestroyBuffer(lveDevice.device(), blasBuffers[chunkPos], nullptr);
            vkFreeMemory(lveDevice.device(), blasMemories[chunkPos], nullptr);
        }

        VkAccelerationStructureGeometryKHR geometry{};
        VkAccelerationStructureBuildRangeInfoKHR rangeInfo{};
        primitiveToGeometry(model, geometry, rangeInfo);

        VkBuffer buffer;
        VkDeviceMemory memory;
        std::cout << "creating BLAS AS\n";

        VkAccelerationStructureKHR blas = createAccelerationStructure(
            VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
            geometry,
            rangeInfo,
            VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR |
                VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR,
            buffer,
            memory);

        std::cout << "BLAS AS created\n";

        blasMap[chunkPos] = blas;
        blasBuffers[chunkPos] = buffer;
        blasMemories[chunkPos] = memory;
        blasDirty = true;
        std::cout << "blas finnished building" << '\n';
    }

    void AccelerationStructure::rebuildTLAS(const std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, IVec3Hash> &chunks)
    {

        if (blasMap.empty())
        {
            std::cout << "no BLAS available\n";
            return;
        }
        std::cout << "rebuild tlas" << '\n';

        // VkTransformMatrixKHR is row-major 3x4, glm::mat4 is column-major
        auto toTransformMatrixKHR = [](const glm::mat4 &m)
        {
            VkTransformMatrixKHR t;
            memcpy(&t, glm::value_ptr(glm::transpose(m)), sizeof(t));
            return t;
        };

        std::cout << "step 1" << '\n';

        // 1. Build instance list from all chunks that have a BLAS
        std::vector<VkAccelerationStructureInstanceKHR> tlasInstances;
        tlasInstances.reserve(chunks.size());

        for (auto &[pos, chunk] : chunks)
        {
            // std::cout << "enter loop" << '\n';
            if (!chunk || !chunk->chunkModel)
                continue;
            if (!blasMap.count(pos))
                continue; // BLAS not built yet

            // Get the BLAS device address
            VkAccelerationStructureDeviceAddressInfoKHR addressInfo{};
            addressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
            addressInfo.accelerationStructure = blasMap[pos];
            uint64_t blasAddress = lveDevice.vkGetAccelerationStructureDeviceAddressKHR(
                lveDevice.device(), &addressInfo);

            VkAccelerationStructureInstanceKHR instance{};
            std::cout << "building instance at "
                      << pos.x << ", "
                      << pos.y << ", "
                      << pos.z << std::endl;

            auto transform = chunk->mat4();

            std::cout << "got transform\n";

            instance.transform = toTransformMatrixKHR(transform);

            std::cout << "converted transform\n";
            instance.instanceCustomIndex = 0;
            instance.accelerationStructureReference = blasAddress;
            instance.instanceShaderBindingTableRecordOffset = 0;
            instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
            instance.mask = 0xFF;
            tlasInstances.emplace_back(instance);
        }

        if (tlasInstances.empty())
            return;

        std::cout << "step 2" << '\n';

        // 2. Upload instance buffer
        VkDeviceSize instanceBufferSize = sizeof(VkAccelerationStructureInstanceKHR) * tlasInstances.size();

        // Staging buffer
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingMemory;
        lveDevice.createBuffer(
            instanceBufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingMemory);

        void *data;
        vkMapMemory(lveDevice.device(), stagingMemory, 0, instanceBufferSize, 0, &data);
        memcpy(data, tlasInstances.data(), instanceBufferSize);
        vkUnmapMemory(lveDevice.device(), stagingMemory);

        // Device-local instance buffer
        VkBuffer instanceBuffer;
        VkDeviceMemory instanceMemory;
        lveDevice.createBuffer(
            instanceBufferSize,
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            instanceBuffer,
            instanceMemory);

        lveDevice.copyBuffer(stagingBuffer, instanceBuffer, instanceBufferSize);

        vkDestroyBuffer(lveDevice.device(), stagingBuffer, nullptr);
        vkFreeMemory(lveDevice.device(), stagingMemory, nullptr);
        std::cout << "step 3" << '\n';

        // 3. Destroy old TLAS
        if (tlas != VK_NULL_HANDLE)
        {
            lveDevice.vkDestroyAccelerationStructureKHR(lveDevice.device(), tlas, nullptr);
            vkDestroyBuffer(lveDevice.device(), tlasBuffer, nullptr);
            vkFreeMemory(lveDevice.device(), tlasMemory, nullptr);
            tlas = VK_NULL_HANDLE;
            tlasBuffer = VK_NULL_HANDLE;
            tlasMemory = VK_NULL_HANDLE;
        }

        // 4. Build TLAS geometry
        std::cout << "build tlas" << '\n';
        VkAccelerationStructureGeometryInstancesDataKHR geometryInstances{};
        geometryInstances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
        geometryInstances.data.deviceAddress = lveDevice.getBufferDeviceAddress(instanceBuffer);

        VkAccelerationStructureGeometryKHR geometry{};
        geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        geometry.geometry = {.instances = geometryInstances};

        VkAccelerationStructureBuildRangeInfoKHR rangeInfo{};
        rangeInfo.primitiveCount = static_cast<uint32_t>(tlasInstances.size());

        tlas = createAccelerationStructure(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, geometry, rangeInfo,
                                           VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
                                           tlasBuffer,
                                           tlasMemory);

        // 5. Cleanup instance buffer
        vkDestroyBuffer(lveDevice.device(), instanceBuffer, nullptr);
        vkFreeMemory(lveDevice.device(), instanceMemory, nullptr);
    }

    bool AccelerationStructure::needsTLASUpdate() const
    {
        return blasDirty;
    }

    void AccelerationStructure::clearTLASDirty()
    {
        blasDirty = false;
    }
}