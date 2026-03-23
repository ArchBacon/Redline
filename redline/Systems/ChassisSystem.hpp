#pragma once

#include <imgui/IconsFontAwesome.h>

#include "core/ecs.hpp"
#include "tools/inspectable.hpp"

class ChassisSystem : public bee::System, public bee::IPanel
{
public:
    ChassisSystem();
    ~ChassisSystem() override = default;
    void Update(float dt) override;
    
    void OnPanel() override;
    [[nodiscard]] std::string GetName() const override { return "Chassis System"; }
    [[nodiscard]] std::string GetIcon() const override { return ICON_FA_CAR; }
};
