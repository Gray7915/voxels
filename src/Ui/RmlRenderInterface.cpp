#include "RmlRenderInterface.hpp"
#include "Util/Types.hpp"
#include <iostream>
#include <Rendering/Core/lve_Texture.hpp>
#include "Rendering/Systems/RmlRenderSystem.hpp"
#include <stb_image.h>
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
        // std::cout << "CompileGeometry called with " << vertices.size() << " vertices\n";
        return handle;
    }

    void RmlRenderInterface::RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) {
        auto it = geometryMap.find(geometry);
        if (it == geometryMap.end() || activeCommandBuffer == VK_NULL_HANDLE || pipeline == VK_NULL_HANDLE)
            return;

        auto &geom = it->second;

        vkCmdBindPipeline(activeCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        int useTexture = 0;
        VkDescriptorSet descToBind = fallbackDescriptorSet;
        if (texture != 0) {
            auto texIt = textureMap.find(texture);
            if (texIt != textureMap.end()) {
                descToBind = texIt->second.descriptorSet;
                useTexture = texIt->second.isFont ? 1 : 2;
            }
        }
        vkCmdBindDescriptorSets(activeCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descToBind, 0, nullptr);

        VkBuffer buffers[] = {geom.vertexBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(activeCommandBuffer, 0, 1, buffers, offsets);
        vkCmdBindIndexBuffer(activeCommandBuffer, geom.indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);

        RmlPushConstants push{{translation.x, translation.y}, {(float)viewportWidth, (float)viewportHeight}, useTexture};
        vkCmdPushConstants(activeCommandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(RmlPushConstants), &push);

        vkCmdDrawIndexed(activeCommandBuffer, geom.indexCount, 1, 0, 0, 0);
    }

    void RmlRenderInterface::ReleaseGeometry(Rml::CompiledGeometryHandle geometry) {
        auto it = geometryMap.find(geometry);
        if (it == geometryMap.end())
            return;

        auto vb = std::shared_ptr<lve::LveBuffer>(std::move(it->second.vertexBuffer));
        auto ib = std::shared_ptr<lve::LveBuffer>(std::move(it->second.indexBuffer));

        device.queueDeletion([vb, ib]() {
        }, currentFrameIndex);

        geometryMap.erase(it);
    }

    void RmlRenderInterface::ReleaseTexture(Rml::TextureHandle texture) {
        auto it = textureMap.find(texture);
        if (it == textureMap.end())
            return;

        auto tex = std::shared_ptr<lve::LveTexture>(std::move(it->second.texture));

        device.queueDeletion([tex]() {
        }, currentFrameIndex);

        textureMap.erase(it);
    }

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

    Rml::TextureHandle RmlRenderInterface::createTexture(const unsigned char *data, int width, int height, bool isFont) {
        auto texture = std::make_unique<lve::LveTexture>(device, data, width, height);
        VkDescriptorSet descriptorSet = rmlRenderSystem->createTextureDescriptor(texture->getImageView(), texture->getSampler());
        Rml::TextureHandle handle = nextTextureHandle++;
        textureMap[handle] = {std::move(texture), descriptorSet, isFont};
        return handle;
    }

    Rml::TextureHandle RmlRenderInterface::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) {
        return createTexture((const unsigned char *)source.data(), source_dimensions.x, source_dimensions.y, true);
    }

    Rml::TextureHandle RmlRenderInterface::LoadTexture(Rml::Vector2i &texture_dimensions, const Rml::String &source) {
        int width, height, channels;
        unsigned char *data = stbi_load(source.c_str(), &width, &height, &channels, 4);
        if (!data)
            return 0;

        texture_dimensions.x = width;
        texture_dimensions.y = height;

        Rml::TextureHandle handle = createTexture(data, width, height, false);
        stbi_image_free(data);
        return handle;
    }
} // namespace lve
