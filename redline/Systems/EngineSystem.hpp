#pragma once

#include <imgui/IconsFontAwesome.h>

#include "core/ecs.hpp"
#include "tools/inspectable.hpp"

class EngineSystem : public bee::System, public bee::IPanel
{
public:
    EngineSystem() = default;
    ~EngineSystem() override = default;
    void Update(float dt) override;
    
    void OnPanel() override;
    [[nodiscard]] std::string GetName() const override { return "Engine System"; }
    [[nodiscard]] std::string GetIcon() const override { return ICON_FA_BOLT; }
};
