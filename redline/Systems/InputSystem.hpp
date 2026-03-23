#pragma once

#include <imgui/IconsFontAwesome.h>

#include "core/ecs.hpp"
#include "tools/inspectable.hpp"

class InputSystem : public bee::System, public bee::IPanel
{
public:
    InputSystem() = default;
    ~InputSystem() override = default;
    void Update(float dt) override;
    
    void OnPanel() override;
    [[nodiscard]] std::string GetName() const override { return "Input System"; }
    [[nodiscard]] std::string GetIcon() const override { return ICON_FA_GAMEPAD; }
};
