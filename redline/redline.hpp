#pragma once

#include <imgui/IconsFontAwesome.h>

#include "core/ecs.hpp"
#include "tools/inspectable.hpp"

struct Active {};

class Redline : public bee::System, public bee::IPanel
{
    float speed = 0.0f;
    bee::Entity camera{entt::null};
    
public:
    Redline();
    ~Redline() override = default;
    void Update(float dt) override;
    
    void OnPanel() override;
    [[nodiscard]] std::string GetName() const override { return "Vehicle"; }
    [[nodiscard]] std::string GetIcon() const override { return ICON_FA_CAR; }
};
