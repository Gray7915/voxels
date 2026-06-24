// highlight_render_system.hpp
#pragma once
#include "lve_device.hpp"
#include "lve_pipeline.hpp"
#include "lve_frame_info.hpp"
#include "lve_model.hpp"
#include "lve_util.hpp"
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
        void render(FrameInfo &frameInfo, bool hasHit, glm::ivec3 blockPos, glm::vec3 direction);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        LveDevice &lveDevice;
        std::unique_ptr<LvePipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
        std::unique_ptr<LveModel> cubeModel;

        // Creates a unit wireframe cube (0,0,0) to (1,1,1), as line-list vertices.
        std::unique_ptr<LveModel> createWireframeCubeModel(LveDevice &device)
        {
            LveModel::Builder builder{};

            // 8 corners of unit cube
            std::array<glm::vec3, 8> corners = {
                glm::vec3{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}, glm::vec3{0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}};

            // 12 edges as pairs of corner indices
            std::array<std::pair<int, int>, 12> edges = {{
                {0, 1}, {1, 2}, {2, 3}, {3, 0}, // bottom face
                {4, 5},
                {5, 6},
                {6, 7},
                {7, 4}, // top face
                {0, 4},
                {1, 5},
                {2, 6},
                {3, 7} // verticals
            }};

            builder.vertices.clear();
            builder.indices.clear();
            for (auto &[a, b] : edges)
            {
                Vertex va{};
                va.position = corners[a];
                va.color = {0.f, 0.f, 0.f}; // black
                builder.vertices.push_back(va);

                Vertex vb{};
                vb.position = corners[b];
                vb.color = {0.f, 0.f, 0.f};
                builder.vertices.push_back(vb);
            }
            // no indices needed; we'll draw as a plain vertex list (24 verts, line list)

            return std::make_unique<LveModel>(device, builder);
        }
    };

}
