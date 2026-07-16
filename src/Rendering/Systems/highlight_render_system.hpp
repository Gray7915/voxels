// highlight_render_system.hpp
#pragma once
#include "Rendering/Core/lve_pipeline.hpp"
#include "Rendering/Core/lve_device.hpp"
#include "Rendering/Core/lve_model.hpp"
#include "Util/IVec3Hash.h"
#include "Util/lve_frame_info.hpp"
#include <memory>

namespace lve
{

    class HighlightRenderSystem
    {
    public:
        HighlightRenderSystem(LveDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~HighlightRenderSystem();

        HighlightRenderSystem(const HighlightRenderSystem &) = delete;
        HighlightRenderSystem &operator=(const HighlightRenderSystem &) = delete;

        // call every frame; hasHit=false skips drawing entirely
        void render(FrameInfo &frameInfo, bool hasHit, glm::ivec3 blockPos, glm::vec3 boxSize);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        LveDevice &lveDevice;
        std::unique_ptr<LvePipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
        std::unique_ptr<LveModel> cubeModel;

        // Creates a unit wireframe cube (0,0,0) to (1,1,1), as line-list vertices.

        std::unique_ptr<LveModel> createOutlineModel(LveDevice &device, const glm::vec3 &size, float thickness = 0.02f)
        {

            const float expand = 0.002f;

            const float minX = -size.x * 0.5f - expand;
            const float maxX = size.x * 0.5f + expand;

            const float minY = -expand;
            const float maxY = size.y + expand;

            const float minZ = -size.z * 0.5f - expand;
            const float maxZ = size.z * 0.5f + expand;

            LveModel::Builder builder{};

            const glm::vec3 color{0.f, 0.f, 0.f};

            auto addBox = [&](const glm::vec3 &min, const glm::vec3 &max)
            {
                uint32_t base = static_cast<uint32_t>(builder.vertices.size());

                glm::vec3 p[8] = {
                    {min.x, min.y, min.z},
                    {max.x, min.y, min.z},
                    {max.x, max.y, min.z},
                    {min.x, max.y, min.z},
                    {min.x, min.y, max.z},
                    {max.x, min.y, max.z},
                    {max.x, max.y, max.z},
                    {min.x, max.y, max.z},
                };

                for (const auto &pos : p)
                {
                    builder.vertices.push_back({pos, color});
                }

                static constexpr uint32_t indices[] = {
                    0, 1, 2, 2, 3, 0,
                    4, 6, 5, 6, 4, 7,
                    0, 4, 5, 5, 1, 0,
                    3, 2, 6, 6, 7, 3,
                    1, 5, 6, 6, 2, 1,
                    0, 3, 7, 7, 4, 0};

                for (uint32_t i : indices)
                    builder.indices.push_back(base + i);
            };

            const float t = thickness;

            // X edges
            addBox({minX, minY, minZ}, {maxX, minY + t, minZ + t});
            addBox({minX, maxY - t, minZ}, {maxX, maxY, minZ + t});
            addBox({minX, minY, maxZ - t}, {maxX, minY + t, maxZ});
            addBox({minX, maxY - t, maxZ - t}, {maxX, maxY, maxZ});

            // Y edges
            addBox({minX, minY, minZ}, {minX + t, maxY, minZ + t});
            addBox({maxX - t, minY, minZ}, {maxX, maxY, minZ + t});
            addBox({minX, minY, maxZ - t}, {minX + t, maxY, maxZ});
            addBox({maxX - t, minY, maxZ - t}, {maxX, maxY, maxZ});

            // Z edges
            // Runs along Z at each corner
            addBox({minX, minY, minZ}, {minX + t, minY + t, maxZ});
            addBox({maxX - t, minY, minZ}, {maxX, minY + t, maxZ});
            addBox({minX, maxY - t, minZ}, {minX + t, maxY, maxZ});
            addBox({maxX - t, maxY - t, minZ}, {maxX, maxY, maxZ});

            return std::make_unique<LveModel>(device, builder);
        }
    };
}
