#pragma once

#include "App/RenderSetup.hpp"
#include "App/SetupECS.hpp"
#include "Rendering/Core/lve_window.hpp"
#include "Rendering/Scene/Scene.hpp"
#include "Rendering/Scene/SceneManager.hpp"
#include "Rendering/Systems/chunk_render_system.hpp"
#include "Rendering/Systems/highlight_render_system.hpp"
#include "Rendering/Systems/simple_render_system.hpp"
#include "Rendering/Systems/ui_render_system.hpp"
#include "Rendering/Systems/OffScreenRenderSystem.hpp"

#include "Rendering/Systems/RmlRenderSystem.hpp"
#include "Rendering/Core/ChunkStagingPool.hpp"
#include "Ui/ImguiManager.hpp"
#include "Util/lve_frame_info.hpp"
#include "Util/AppContext.hpp"
#include "World/Area.hpp"
#include "World/Generation/ChunkState.hpp"

#include "ECS/EntityFactory.hpp"
#include "ECS/SpawnInfo.hpp"

#include "imgui.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <RmlUi/Core.h>
#include "Ui/RmlSystemInterface.hpp"
#include "Ui/RmlRenderInterface.hpp"

#include "Ui/InventoryUI.hpp"
#include "Inventory/ItemStack.hpp"
#include "Inventory/ItemRegistry.hpp"
namespace Rendering
{
    enum class GameSceneState { Loading, Playing };
    class GameScene : public Scene {
      public:
        GameScene(AppContext &context, SceneManager &sceneManager, u64 seed) : context(context), sceneManager(sceneManager), seed(seed) {}

        void onEnter() override {
            chunkRenderSystem.emplace(context.device, context.renderer.getSwapChainRenderPass(), context.renderSetup.globalSetLayout->getDescriptorSetLayout());
            highlightRenderSystem.emplace(context.device, context.renderer.getSwapChainRenderPass(), context.renderSetup.globalSetLayout->getDescriptorSetLayout());
            simpleRenderSystem.emplace(context.device, context.renderer.getSwapChainRenderPass(), context.renderSetup.globalSetLayout->getDescriptorSetLayout());
            offScreenRenderSystem.emplace(context.device, context.renderer.OffScreenUIPass->getRenderPass(), context.renderSetup.globalSetLayout->getDescriptorSetLayout());

            area = std::make_unique<lve::Area>(context.device, seed, context.coordinator, context.stagingPool);
            systemInterface = std::make_unique<RmlSystemInterface>();
            renderInterface = std::make_unique<lve::RmlRenderInterface>(context.device);
            renderInterface->setBlockPreviewResolver([this](BlockId id) -> VkDescriptorSet {
                auto *pass = context.renderer.OffScreenUIPass.get();
                return pass ? pass->getDescriptorSet(id) : VK_NULL_HANDLE;
            });
            Rml::SetSystemInterface(systemInterface.get());
            Rml::SetRenderInterface(renderInterface.get());
            Rml::Initialise();
            Rml::LoadFontFace("../Content/ui/LatoLatin-Regular.ttf");

            rmlContext = Rml::CreateContext("main", Rml::Vector2i(context.window.getExtent().width, context.window.getExtent().height));
            context.window.setRmlContext(rmlContext);
            rmlRenderSystem = std::make_unique<lve::RmlRenderSystem>(context.device, context.renderer.StandaloneUIRenderPass->getRenderPass(), *renderInterface);
            renderInterface->setViewportSize(context.window.getExtent().width, context.window.getExtent().height);
            inventoryUI = std::make_unique<UI::InventoryUI>("Inventory 1", Rml::Vector2f(50, 200), rmlContext);
            std::cout << "make invent thingie" << '\n';
            // inventoryUI->AddItem("Mk III L.A.S.E.R.");
            std::cout << "added big lazer" << '\n';

            // Pre-build all block models — safe here, no frame in flight
            // Pre-create targets
            for (auto &item : inventoryUI->getItems()) {
                BlockId id = std::stoi(item.id);
                context.renderer.OffScreenUIPass->createPreviewForBlock(id);
            }

            // Record and submit all preview renders in one shot before the game loop
            VkCommandBuffer cmd = context.device.beginSingleTimeCommands(context.device.getCommandPool());

            FrameInfo previewFrame{};
            previewFrame.commandBuffer = cmd;
            previewFrame.globalDescriptorSet = context.renderSetup.globalDescriptorSets[0];

            for (auto &item : inventoryUI->getItems()) {
                BlockId id = std::stoi(item.id);
                context.renderer.OffScreenUIPass->begin(cmd, id);
                offScreenRenderSystem->renderInvItems(previewFrame, id);
                context.renderer.OffScreenUIPass->end(cmd);
            }

            context.device.endSingleTimeCommands(cmd, context.device.getCommandPool());

            for (auto &item : inventoryUI->getItems()) {
                BlockId id = std::stoi(item.id);
                VkDescriptorSet ds = context.renderer.OffScreenUIPass->getDescriptorSet(id);
                if (ds != VK_NULL_HANDLE)
                    inventoryUI->AddItemWithImage("slot_" + item.id, id);
            }
        }

        void onExit() override {
            if (rmlContext) {
                rmlContext->UnloadAllDocuments();
                Rml::RemoveContext(rmlContext->GetName());
                rmlContext = nullptr;
            }

            Rml::Shutdown();

            if (pendingUpload.has_value()) {
                vkWaitForFences(context.device.device(), 1, &pendingUpload->fence, VK_TRUE, UINT64_MAX);
                vkFreeCommandBuffers(context.device.device(), context.device.getCommandPool(), 1, &pendingUpload->cmd);
                vkDestroyFence(context.device.device(), pendingUpload->fence, nullptr);
                pendingUpload.reset();
            }
        }

        void update(float deltaTime) override {

            /*
            Need to add some list of things like saplings, crops, ect, that have a timer who decreases by delta time giving the same grown time right?
            Each chunk has its own list so if unloaded when chunk saving added the list should be saved with the chunk. or they could be used to keep chunks loaded? im not sure.
            */
            lastFrameTime = deltaTime;
            // deltaTime = std::min(deltaTime, 0.25f);
            accumulator += deltaTime; // ← only once

            if (rmlContext)
                rmlContext->Update();

            // Input sampled once per render frame, not per physics step
            context.systems.inputSystem->Update(&context.window);
            context.systems.movementSystem->UpdateCamera();

            while (accumulator >= FIXED_DT) {
                if (!inventoryOpen) {
                    context.systems.movementSystem->Update(FIXED_DT, frameIndex);
                }
                context.systems.physicsSystem->Update(FIXED_DT);
                context.systems.collisionSystem->Update(FIXED_DT, *area);
                accumulator -= FIXED_DT;
            }
            context.systems.interactionSystem->Update(deltaTime, context.window, context.device, *area, context.coordinator);
            context.systems.inventorySystem->Update(*area);
            area->updateArea();
            if (state == GameSceneState::Loading) {
                return;
            }
            auto &camTransform = context.coordinator.GetComponent<Transform>(mainCamera);
            camPosition = camTransform.position;
            camRotation = camTransform.rotation;

            hoveredID = context.systems.interactionSystem->hoveredID;

            context.coordinator.eventBus.blockBreakRequest.clear();
            context.coordinator.eventBus.blockPlaceRequested.clear();

            const auto &playerInputComp = context.coordinator.GetComponent<InputComponent>(mainCamera);

            static bool EWasPressed = false;
            bool EIsPressed = playerInputComp.openInventory;
            if (EWasPressed && !EIsPressed) {
                EWasPressed = false;
            }

            if (!EWasPressed && EIsPressed) {
                EWasPressed = true;
                inventoryUI->openCreativeInventory();
                inventoryOpen = !inventoryOpen;
                context.window.setMouseActive();
            }
        }

        void render(lve::FrameInfo &frameInfo) override {
            for (auto &[slotIndex, blockId] : pendingHotbarUpdates) {
                inventoryUI->AddItemWithImage("slot_" + std::to_string(slotIndex), blockId);
            }
            pendingHotbarUpdates.clear();

            VkExtent2D extent = context.window.getExtent();
            if (extent.width != lastWidth || extent.height != lastHeight) {
                lastWidth = extent.width;
                lastHeight = extent.height;
                if (rmlContext)
                    rmlContext->SetDimensions(Rml::Vector2i(extent.width, extent.height));
                if (renderInterface)
                    renderInterface->setViewportSize(extent.width, extent.height);
            }

            area->tick(context.device, camPosition, frameInfo.frameIndex);
            uploadPendingChunks(frameInfo, 4);

            if (state == GameSceneState::Loading) {
                if (chunksReady() >= minimumChunksToLoad) {
                    state = GameSceneState::Playing;
                    context.window.setMouseActive();
                    vec3 spawnPos = findSpawnPosition({0, 0});
                    camPosition = spawnPos;
                    mainCamera = ECS::EntityFactory::Get().Create("MainCamera", ECS::SpawnInfo{.position = spawnPos, .scale{1, 1, 1}});
                    inventoryUI->onItemClicked = [this](const Rml::String &itemId) {
                        auto &inv = context.coordinator.GetComponent<InventoryComponent>(mainCamera);
                        bool added = false;
                        for (auto &stack : inv.inventoryStacks) {
                            if (!stack.has_value()) {
                                stack = ItemStack(ItemRegistry::Get().GetItemByID(std::stoi(itemId)), ItemRegistry::Get().GetItemByID(std::stoi(itemId))->maxStackSize);
                                added = true;
                                const int blockId = std::stoi(itemId);
                                pendingHotbarUpdates.push_back({blockId, blockId});
                                break;
                            }
                        }
                        if (!added) {
                            inv.inventoryStacks.at(0) = lve::ItemStack(ItemRegistry::Get().GetItemByID(std::stoi(itemId)), 1);
                        }
                    };
                } else {
                    renderLoadingScreen(frameInfo);
                    return;
                }
            }

            auto &camTransform = context.coordinator.GetComponent<Transform>(mainCamera);
            auto &camera = context.coordinator.GetComponent<CameraComponent>(mainCamera);

            lve::GlobalUbo ubo{};
            ubo.projectionView = camera.projectionMatrix * camera.viewMatrix;
            ubo.cameraPosition = ivec4(camTransform.position, 1.f);
            ubo.fogColor = glm::vec4(0.4f, 0.7f, 1.0f, 1.0f);
            ubo.fogStart = 200.0f;
            ubo.fogEnd = 228.0f;

            // Rendering — completely unaffected by upload state
            context.renderer.geometryPass->begin(frameInfo.commandBuffer, context.renderer.getImageIndex());
            chunkRenderSystem->renderChunks(frameInfo, area->chunks, context.coordinator, mainCamera);
            if (hoveredID.w != 0) {
                auto block = lve::BlockRegistry::Get().GetBlockByID(hoveredID.w);
                vec3 boxSize{1, 1, 1};
                if (block)
                    boxSize = block->highlightBoxSize;
                vec3 forward = {cos(camRotation.x) * sin(camRotation.y), -sin(camRotation.x), cos(camRotation.x) * cos(camRotation.y)};
                vec3 dir = glm::normalize(forward);
                highlightRenderSystem->render(frameInfo, hoveredID.w, hoveredID, boxSize, dir);
            }
            context.renderer.geometryPass->end(frameInfo.commandBuffer);

            for (auto &item : inventoryUI->getItems()) {
                BlockId id = std::stoi(item.id);
                // std::cout << "into for loop" << '\n';
                if (!context.renderer.OffScreenUIPass->hasPreview(id)) {
                    std::cout << "Rendering preview for block " << id << '\n';
                    context.renderer.OffScreenUIPass->createPreviewForBlock(id);

                    std::cout << "Has preview after create: " << context.renderer.OffScreenUIPass->hasPreview(id) << '\n';

                    context.renderer.OffScreenUIPass->begin(frameInfo.commandBuffer, id);
                    offScreenRenderSystem->renderInvItems(frameInfo, id);
                    context.renderer.OffScreenUIPass->end(frameInfo.commandBuffer);

                    VkDescriptorSet ds = context.renderer.OffScreenUIPass->getDescriptorSet((int)id);
                    std::cout << "Descriptor set after render: " << ds << '\n';

                    // Register with RmlUi immediately after rendering
                    inventoryUI->AddItemWithImage("slot_" + std::to_string((int)id), id);
                }
            }

            renderUI(frameInfo, camTransform, camera);

            context.renderSetup.uboBuffers[frameInfo.frameIndex]->writeToBuffer(&ubo);
            context.renderSetup.uboBuffers[frameInfo.frameIndex]->flush();
            RenderStats::Get().reset();
        }

      private:
        static constexpr int minimumChunksToLoad = 9;

        void uploadPendingChunks(lve::FrameInfo &frameInfo, int budget) {
            VkCommandBuffer cmd = frameInfo.commandBuffer;
            auto &device = context.device;
            int frame = (int)device.getFrameCount();
            bool recorded = false;

            for (auto &[coord, chunkPtr] : area->chunks) {
                if (budget <= 0)
                    break;
                lve::Chunk *chunk = chunkPtr.get();
                if (!chunk || !chunk->pendingMesh)
                    continue;

                lve::ChunkMeshData mesh = std::move(*chunk->pendingMesh);
                chunk->pendingMesh.reset();

                if (mesh.indices.empty()) { // all-air chunk
                    if (chunk->chunkState == ChunkState::PendingUpload)
                        chunk->chunkState = ChunkState::Uploaded;
                    continue;
                }
                --budget;

                VkDeviceSize vSize = sizeof(lve::Vertex) * mesh.vertices.size();
                VkDeviceSize iSize = sizeof(u32) * mesh.indices.size();

                auto staging = std::make_shared<lve::LveBuffer>(device, vSize + iSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                staging->map();
                staging->writeToBuffer(mesh.vertices.data(), vSize, 0);
                staging->writeToBuffer(mesh.indices.data(), iSize, vSize);
                staging->unmap();

                auto vertexDst = std::make_unique<lve::LveBuffer>(device, sizeof(lve::Vertex), (u32)mesh.vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                auto indexDst = std::make_unique<lve::LveBuffer>(device, sizeof(u32), (u32)mesh.indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                VkBufferCopy vc{0, 0, vSize};
                VkBufferCopy ic{vSize, 0, iSize};
                vkCmdCopyBuffer(cmd, staging->getBuffer(), vertexDst->getBuffer(), 1, &vc);
                vkCmdCopyBuffer(cmd, staging->getBuffer(), indexDst->getBuffer(), 1, &ic);

                device.queueDeletion([staging]() {
                }, frame); // keep staging alive until GPU is done

                chunk->applyMesh(std::make_unique<lve::LveModel>(device, std::move(vertexDst), std::move(indexDst)), frame, device);
                recorded = true;
            }

            if (recorded) {
                VkMemoryBarrier b{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
                b.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                b.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT;
                vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 1, &b, 0, nullptr, 0, nullptr);
            }
        }

        int chunksReady() {
            auto spawnChunk = area->chunks.find(ivec3{0, 0, 0});
            if (spawnChunk != area->chunks.end()) {
                // std::cout << "spawn chunk not loaded" << '\n';
                if (!spawnChunk->second->voxelData.isGenerated()) {
                    return 0;
                }
            }

            int count = 0;
            // std::cout << "chunks ready " << count << '\n';
            for (auto &[coord, chunk] : area->chunks) {
                if (chunk && chunk->chunkState == lve::ChunkState::Uploaded && chunk->chunkModel != nullptr)
                    count++;
                // if (chunk->chunkModel == nullptr)
                //  std::cout << "chunk model null" << '\n';
            }
            return count;
        }

        vec3 findSpawnPosition(vec2 xzPos) {
            for (int y = 127; y >= 0; y--) {
                if (area->isBlockSolid({xzPos.x, y, xzPos.y})) {
                    return vec3{xzPos.x + 0.5f, y + 2, xzPos.y + 0.5f};
                }
            }
            return vec3{xzPos.x, 68, xzPos.y};
        }

        void renderLoadingScreen(lve::FrameInfo &frameInfo) {
            context.renderer.StandaloneUIRenderPass->begin(frameInfo.commandBuffer, context.renderer.getImageIndex());
            context.imgui.newFrame();

            ImVec2 displaySize = ImGui::GetIO().DisplaySize;

            ImVec2 windowSize(300.0f, 80.0f);

            ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
            ImGui::SetNextWindowPos(ImVec2((displaySize.x - windowSize.x) * 0.5f, (displaySize.y - windowSize.y) * 0.5f), ImGuiCond_Always);

            ImGui::Begin("##loading", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground);
            ImGui::Text("Generating world... %d / %d", chunksReady(), minimumChunksToLoad);
            ImGui::End();

            context.imgui.render(frameInfo.commandBuffer);
            context.renderer.StandaloneUIRenderPass->end(frameInfo.commandBuffer);
        }

        void renderUI(lve::FrameInfo &frameInfo, Transform &camTransform, CameraComponent &camera) {
            context.renderer.UiRenderPass->begin(frameInfo.commandBuffer, context.renderer.getImageIndex());
            rmlRenderSystem->render(frameInfo.commandBuffer);

            if (rmlContext) {
                renderInterface->setFrameIndex(context.device.getFrameCount());
                rmlContext->Render();
            }
            context.imgui.newFrame();
            context.imgui.drawDebugWindow(lastFrameTime, camPosition, camTransform, camera, *area);
            context.imgui.drawCrosshair(context.window.getExtent().width, context.window.getExtent().height);
            context.imgui.drawInv(context.coordinator.GetComponent<InventoryComponent>(mainCamera));
            context.imgui.render(frameInfo.commandBuffer);
            context.renderer.UiRenderPass->end(frameInfo.commandBuffer);
        }

        AppContext &context;
        SceneManager &sceneManager;
        u64 seed;

        std::unique_ptr<lve::Area> area;
        Entity mainCamera;

        std::optional<lve::ChunkRenderSystem> chunkRenderSystem;
        std::optional<lve::HighlightRenderSystem> highlightRenderSystem;
        std::optional<lve::SimpleRenderSystem> simpleRenderSystem;
        std::optional<lve::OffScreenRenderSystem> offScreenRenderSystem;

        std::unique_ptr<RmlSystemInterface> systemInterface;
        std::unique_ptr<lve::RmlRenderInterface> renderInterface;
        std::unique_ptr<lve::RmlRenderSystem> rmlRenderSystem;

        std::unique_ptr<UI::InventoryUI> inventoryUI;
        Rml::Context *rmlContext = nullptr;
        Rml::ElementDocument *document = nullptr;

        bool inventoryOpen = false;
        bool ePressedLastFrame = false;

        ivec4 hoveredID{0};
        vec3 camPosition{0};
        vec3 camRotation{0};
        float lastFrameTime{0};

        GameSceneState state = GameSceneState::Loading;

        struct PendingUpload {
            VkFence fence;
            VkCommandBuffer cmd;
            std::vector<ChunkStagingPool::Slot> slots;
        };
        std::optional<PendingUpload> pendingUpload;

        int prevFrame = -1;

        u32 lastHeight = 0;
        u32 lastWidth = 0;

        const float FIXED_DT = 1.0f / 60.0f;
        float accumulator = 0.0f;
        u64 frameIndex = 0;

        std::vector<std::pair<int, BlockId>> pendingHotbarUpdates;
    };
} // namespace Rendering
