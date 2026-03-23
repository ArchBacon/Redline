#pragma once

#include <imgui/IconsFontAwesome.h>

#include "core/ecs.hpp"
#include "tools/inspectable.hpp"

class SteeringSystem : public bee::System, public bee::IPanel
{
public:
    SteeringSystem() = default;
    ~SteeringSystem() override = default;
    void Update(float dt) override;
    
    void OnPanel() override;
    [[nodiscard]] std::string GetName() const override { return "Steering System"; }
    [[nodiscard]] std::string GetIcon() const override { return ICON_FA_COMPASS; }
};
