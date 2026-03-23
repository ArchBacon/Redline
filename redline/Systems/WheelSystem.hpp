#pragma once

#include <imgui/IconsFontAwesome.h>

#include "core/ecs.hpp"
#include "tools/inspectable.hpp"

class WheelSystem : public bee::System, public bee::IPanel
{
public:
    WheelSystem() = default;
    ~WheelSystem() override = default;
    void Update(float dt) override;
    
    void OnPanel() override;
    [[nodiscard]] std::string GetName() const override { return "Wheel System"; }
    [[nodiscard]] std::string GetIcon() const override { return ICON_FA_WHEELCHAIR; }
};
