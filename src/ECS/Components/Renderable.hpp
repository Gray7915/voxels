#pragma once
#include <memory>
#include "Rendering/Core/lve_model.hpp"

struct RenderableComponent
{
    std::shared_ptr<lve::LveModel> model;
};
