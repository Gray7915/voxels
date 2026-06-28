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

            constexpr float inset = 1.f / 16.f;
            constexpr float scale = 1.01f; // slightly larger than the block to avoid z-fighting
            const glm::vec3 center{0.5f, 0.5f, 0.5f};

            // 8 corners of unit cube
            std::array<glm::vec3, 8> corners = {
                glm::vec3{0, 0, 0}, glm::vec3{1, 0, 0}, glm::vec3{1, 1, 0}, glm::vec3{0, 1, 0},
                glm::vec3{0, 0, 1}, glm::vec3{1, 0, 1}, glm::vec3{1, 1, 1}, glm::vec3{0, 1, 1}};

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

            glm::vec3 color{1.f, 1.f, 1.f};

            auto addTri = [&](const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c)
            {
                Vertex va{};
                va.position = center + (a - center) * scale;
                va.color = color;
                builder.vertices.push_back(va);
                Vertex vb{};
                vb.position = center + (b - center) * scale;
                vb.color = color;
                builder.vertices.push_back(vb);
                Vertex vc{};
                vc.position = center + (c - center) * scale;
                vc.color = color;
                builder.vertices.push_back(vc);
            };

            for (auto &[ia, ib] : edges)
            {
                glm::vec3 pa = corners[ia];
                glm::vec3 pb = corners[ib];
                glm::vec3 edgeDir = pb - pa;

                int edgeAxis = 0;
                float maxAbs = std::abs(edgeDir.x);
                if (std::abs(edgeDir.y) > maxAbs)
                {
                    edgeAxis = 1;
                    maxAbs = std::abs(edgeDir.y);
                }
                if (std::abs(edgeDir.z) > maxAbs)
                {
                    edgeAxis = 2;
                    maxAbs = std::abs(edgeDir.z);
                }

                int o1 = (edgeAxis + 1) % 3;
                int o2 = (edgeAxis + 2) % 3;

                float valO1 = pa[o1];
                float valO2 = pa[o2];

                glm::vec3 nA{0.f};
                nA[o2] = (valO2 == 0.f) ? 1.f : -1.f;

                glm::vec3 nB{0.f};
                nB[o1] = (valO1 == 0.f) ? 1.f : -1.f;

                glm::vec3 hingeA = pa;
                glm::vec3 hingeB = pb;
                glm::vec3 flapA0 = pa + inset * nA;
                glm::vec3 flapA1 = pb + inset * nA;
                glm::vec3 flapB0 = pa + inset * nB;
                glm::vec3 flapB1 = pb + inset * nB;

                addTri(hingeA, hingeB, flapA1);
                addTri(hingeA, flapA1, flapA0);

                addTri(hingeA, hingeB, flapB1);
                addTri(hingeA, flapB1, flapB0);
            }

            return std::make_unique<LveModel>(device, builder);
        }
    };

}
