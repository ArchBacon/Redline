#pragma once
#include "core/ecs.hpp"

struct WheelVisual
{
    bee::Entity car  = entt::null;
    bool isFront     = false;
    bool mirror      = false;
    float spinAngle  = 0.0f;  // accumulated rad
};
