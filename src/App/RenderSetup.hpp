#pragma once
#include <memory.h>
#include <glm/glm.hpp>

#include "Rendering/Core/lve_descriptors.hpp"
#include "Rendering/Core/lve_Texture.hpp"
#include "Rendering/Core/lve_buffer.hpp"
#include "Rendering/Core/lve_device.hpp"
#include "Rendering/Core/SwapChain.hpp"

namespace lve
{
    class LveDevice;

    struct RenderSetup
    {
        std::unique_ptr<LveTexture> texture;
        std::unique_ptr<LveDescriptorPool> globalPool;
        std::unique_ptr<LveDescriptorSetLayout> globalSetLayout;
        std::vector<std::unique_ptr<LveBuffer>> uboBuffers;
        std::vector<VkDescriptorSet> globalDescriptorSets;
    };

    RenderSetup setupRender(LveDevice &lveDevice);

    struct GlobalUbo
    {
        glm::mat4 projectionView{1.f};
        // glm::vec3 lightDirection = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
        glm::vec4 ambientLightColor{1.f, 1.f, 1.f, 0.02f};
        glm::vec3 lightPosition{0, 70, 0};
        alignas(16) glm::vec4 lightColor{1.f, 1.f, 1.f, 5.f}; // w is for light intensity
    };
}
