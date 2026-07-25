#pragma once

#include "lve_device.hpp"
#include "lve_buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include "Util/lve_util.hpp"
#include "Util/Types.hpp"

#include <memory>
#include <vector>
#include <cstdint>

namespace lve
{

    struct ModelSection
    {
        std::string name;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
    };

    class LveModel
    {
    public:
        struct Builder
        {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};
            std::unordered_map<std::string, ModelSection> sections;
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

        std::vector<Vertex> modelVerticies{};
        std::vector<u32> modelIindices{};
        std::unordered_map<std::string, ModelSection> modelSections;

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
