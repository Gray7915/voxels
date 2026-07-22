#pragma once

#include "lve_device.hpp"
#include "lve_buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include "Util/lve_util.hpp"

#include <memory>
#include <vector>
#include <cstdint>
#include <iostream>

namespace lve
{
    class LveModel
    {
    public:
        struct Builder
        {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModel(const std::string &filepath);
        };

        LveModel(LveDevice &device, const LveModel::Builder &builder);
        LveModel(LveDevice &device, const LveModel::Builder &builder, VkCommandPool pool);
        LveModel();
        ~LveModel();

        LveModel(const LveModel &) = delete;
        LveModel &operator=(const LveModel &) = delete;

        static std::unique_ptr<LveModel> createModelFromFile(LveDevice &device, const std::string &filepath);
        static std::unique_ptr<LveModel> createChunkModel(LveDevice &device, std::vector<Vertex> vertices, std::vector<uint32_t> indices, VkCommandPool pool);
        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

<<<<<<< HEAD
        // Add to lve_model.hpp public section:
        uint32_t getVertexCount() const { return vertexCount; }
        uint32_t getIndexCount() const { return indexCount; }
        VkBuffer getVertexBuffer() const
        {
            auto buf = vertexBuffer->getBuffer();
            std::cout
                << "MODEL vertexBuffer ptr: "
                << vertexBuffer.get()
                << "\n";

            std::cout
                << "MODEL VkBuffer: "
                << buf
                << "\n";

            return vertexBuffer->getBuffer();
        }
=======
        uint32_t getVertexCount() const { return vertexCount; }
        uint32_t getIndexCount() const { return indexCount; }
        VkBuffer getVertexBuffer() const { return vertexBuffer->getBuffer(); }
>>>>>>> 6b374db (some stuff)
        VkBuffer getIndexBuffer() const { return indexBuffer->getBuffer(); }
        bool hasIndices() const { return hasIndexBuffer; }

    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices, VkCommandPool pool);
        void createIndexBuffer(const std::vector<uint32_t> &indices, VkCommandPool pool);

        LveDevice &lveDevice;

        std::unique_ptr<LveBuffer> vertexBuffer;
        uint32_t vertexCount;

        bool hasIndexBuffer = false;
        std::unique_ptr<LveBuffer> indexBuffer;
        uint32_t indexCount;
    };
}
