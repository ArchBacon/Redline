#pragma once

#include <glm/glm.hpp>

using int2 = glm::ivec2;
using int3 = glm::ivec3;
using int4 = glm::ivec4;

// unsigned integers
using uint2 = glm::uvec2;
using uint3 = glm::uvec3;
using uint4 = glm::uvec4;

// floats
using float2 = glm::vec2;
using float3 = glm::vec3;
using float4 = glm::vec4;

#include <imgui/IconsFontAwesome.h>

#include "core/ecs.hpp"
#include "tools/inspectable.hpp"

struct PlayerCar {};

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
    [[nodiscard]] std::string GetIcon() const override { return ICON_FA_ADN; }
};
