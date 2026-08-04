#include "RmlRenderInterface.hpp"
#include "Util/Types.hpp"
#include <iostream>
#include <Rendering/Core/lve_Texture.hpp>
#include "Rendering/Systems/RmlRenderSystem.hpp"
namespace lve
{
    Rml::CompiledGeometryHandle RmlRenderInterface::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) {
        CompiledGeometry geom{};

        // vertex buffer
        auto vertStaging = std::make_unique<lve::LveBuffer>(device, sizeof(Rml::Vertex), (uint32_t)vertices.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vertStaging->map();
        vertStaging->writeToBuffer((void *)vertices.data());

        geom.vertexBuffer = std::make_unique<lve::LveBuffer>(device, sizeof(Rml::Vertex), (uint32_t)vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        device.copyBuffer(vertStaging->getBuffer(), geom.vertexBuffer->getBuffer(), sizeof(Rml::Vertex) * vertices.size());

        // index buffer
        auto idxStaging = std::make_unique<lve::LveBuffer>(device, sizeof(int), (uint32_t)indices.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        idxStaging->map();
        idxStaging->writeToBuffer((void *)indices.data());

        geom.indexBuffer =
            std::make_unique<lve::LveBuffer>(device, sizeof(int), (uint32_t)indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        device.copyBuffer(idxStaging->getBuffer(), geom.indexBuffer->getBuffer(), sizeof(int) * indices.size());

        geom.indexCount = (uint32_t)indices.size();

        Rml::CompiledGeometryHandle handle = nextHandle++;
        geometryMap[handle] = std::move(geom);
        std::cout << "CompileGeometry called with " << vertices.size() << " vertices\n";
        return handle;
    }

    void RmlRenderInterface::RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) {
        auto it = geometryMap.find(geometry);
        if (it == geometryMap.end() || activeCommandBuffer == VK_NULL_HANDLE || pipeline == VK_NULL_HANDLE)
            return;

        auto &geom = it->second;

        vkCmdBindPipeline(activeCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        // bind texture if present
        if (texture != 0) {
            auto texIt = textureMap.find(texture);
            if (texIt != textureMap.end()) {
                vkCmdBindDescriptorSets(activeCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &texIt->second.descriptorSet, 0, nullptr);
            }
        }

        VkBuffer buffers[] = {geom.vertexBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(activeCommandBuffer, 0, 1, buffers, offsets);
        vkCmdBindIndexBuffer(activeCommandBuffer, geom.indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);

        RmlPushConstants push{{translation.x, translation.y}, {(float)viewportWidth, (float)viewportHeight}, texture != 0 ? 1 : 0};

        vkCmdPushConstants(activeCommandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(RmlPushConstants), &push);

        vkCmdDrawIndexed(activeCommandBuffer, geom.indexCount, 1, 0, 0, 0);
    }

    void RmlRenderInterface::ReleaseGeometry(Rml::CompiledGeometryHandle geometry) { geometryMap.erase(geometry); }

    void RmlRenderInterface::EnableScissorRegion(bool enable) {
        std::cout << "EnableScissorRegion: " << enable << "\n";
        scissorEnabled = enable;
        if (!enable && activeCommandBuffer != VK_NULL_HANDLE) {
            VkRect2D scissor{{0, 0}, {(uint32_t)viewportWidth, (uint32_t)viewportHeight}};
            vkCmdSetScissor(activeCommandBuffer, 0, 1, &scissor);
        }
    }

    void RmlRenderInterface::SetScissorRegion(Rml::Rectanglei region) {
        std::cout << "SetScissorRegion: " << region.Left() << "," << region.Top() << " " << region.Width() << "x" << region.Height() << "\n";
        if (activeCommandBuffer == VK_NULL_HANDLE)
            return;
        VkRect2D scissor{{region.Left(), region.Top()}, {(uint32_t)region.Width(), (uint32_t)region.Height()}};
        vkCmdSetScissor(activeCommandBuffer, 0, 1, &scissor);
    }

    Rml::TextureHandle RmlRenderInterface::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) {
        auto texture = std::make_unique<lve::LveTexture>(device, (const unsigned char *)source.data(), source_dimensions.x, source_dimensions.y);

        VkDescriptorSet descriptorSet = rmlRenderSystem->createTextureDescriptor(texture->getImageView(), texture->getSampler());

        Rml::TextureHandle handle = nextTextureHandle++;
        textureMap[handle] = {std::move(texture), descriptorSet};
        return handle;
    }

    void RmlRenderInterface::ReleaseTexture(Rml::TextureHandle texture) { textureMap.erase(texture); }
} // namespace lve