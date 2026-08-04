#pragma once
#include <RmlUi/Core/RenderInterface.h>
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <memory>
#include "Rendering/Core/lve_buffer.hpp"
#include "Rendering/Core/lve_device.hpp"
#include "Rendering/Core/lve_Texture.hpp"
#include "Rendering/Core/lve_descriptors.hpp"

namespace lve
{
    class RmlRenderSystem;

    class RmlRenderInterface : public Rml::RenderInterface {
      public:
        RmlRenderInterface(lve::LveDevice &device) : device(device) {}

        void beginFrame(VkCommandBuffer commandBuffer) { activeCommandBuffer = commandBuffer; }

        void setPipeline(VkPipeline p, VkPipelineLayout layout) {
            pipeline = p;
            pipelineLayout = layout;
        }

        void setViewportSize(int width, int height) {
            viewportWidth = width;
            viewportHeight = height;
        }

        void setDescriptorLayout(lve::LveDescriptorSetLayout *layout, lve::LveDescriptorPool *pool, lve::RmlRenderSystem *system) { rmlRenderSystem = system; }

        Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override;
        void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) override;
        void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;
        void EnableScissorRegion(bool enable) override;
        void SetScissorRegion(Rml::Rectanglei region) override;
        Rml::TextureHandle LoadTexture(Rml::Vector2i &texture_dimensions, const Rml::String &source) override { return 1; }
        Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) override;
        void ReleaseTexture(Rml::TextureHandle texture) override;

      private:
        struct CompiledGeometry {
            std::unique_ptr<lve::LveBuffer> vertexBuffer;
            std::unique_ptr<lve::LveBuffer> indexBuffer;
            uint32_t indexCount;
        };

        struct RmlTexture {
            std::unique_ptr<lve::LveTexture> texture;
            VkDescriptorSet descriptorSet;
        };

        lve::LveDevice &device;
        VkCommandBuffer activeCommandBuffer = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        int viewportWidth = 800;
        int viewportHeight = 600;
        bool scissorEnabled = false;

        lve::RmlRenderSystem *rmlRenderSystem = nullptr;

        std::unordered_map<Rml::CompiledGeometryHandle, CompiledGeometry> geometryMap;
        std::unordered_map<Rml::TextureHandle, RmlTexture> textureMap;
        uint64_t nextHandle = 1;
        uint64_t nextTextureHandle = 1;
    };
} // namespace lve