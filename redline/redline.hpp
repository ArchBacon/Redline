#pragma once

#include "glm/glm.hpp"
#include "core/ecs.hpp"

class Redline : public bee::System
{
public:
    Redline();
    ~Redline() override = default;
    void Update(float dt) override;
};
