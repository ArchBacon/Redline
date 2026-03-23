#pragma once

#include <imgui/IconsFontAwesome.h>

#include "core/ecs.hpp"
#include "tools/inspectable.hpp"

class GearboxSystem : public bee::System, public bee::IPanel
{
public:
    GearboxSystem() = default;
    ~GearboxSystem() override = default;
    void Update(float dt) override;
    
    void OnPanel() override;
    [[nodiscard]] std::string GetName() const override { return "Gearbox System"; }
    [[nodiscard]] std::string GetIcon() const override { return ICON_FA_COG; }
};
