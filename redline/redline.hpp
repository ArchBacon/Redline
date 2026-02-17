#pragma once

#include "core/ecs.hpp"

struct Active {};

class Redline : public bee::System
{
public:
    Redline();
    ~Redline() override = default;
    void Update(float dt) override;
};
