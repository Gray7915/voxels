#pragma once
#include <memory>
#include "lve_model.hpp"

struct RenderableComponent
{
    std::shared_ptr<lve::LveModel> model;
};
