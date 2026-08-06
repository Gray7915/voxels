#pragma once
#include "Rendering/Core/lve_device.hpp"
#include "Rendering/Core/lve_renderer.hpp"
#include "Ui/ImguiManager.hpp"
#include "App/RenderSetup.hpp"
#include "App/SetupECS.hpp"
#include "Rendering/Core/ChunkStagingPool.hpp"

struct AppContext {
    lve::LveDevice &device;
    lve::LveWindow &window;
    lve::LveRenderer &renderer;
    lve::ImguiManager &imgui;
    lve::RenderSetup &renderSetup;
    lve::Coordinator &coordinator;
    lve::ECSSystems &systems;
    ChunkStagingPool &stagingPool;
};
