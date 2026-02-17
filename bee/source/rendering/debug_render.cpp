#include "rendering/debug_render.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace bee;
using namespace glm;

void DebugRenderer::AddCircle(DebugCategory::Enum category,
                              const vec3& center,
                              float radius,
                              const glm::vec3& normal,
                              const vec4& color)
{
    if (!(m_categoryFlags & category)) return;

    constexpr float dt = glm::two_pi<float>() / 32.0f;
    float t = 0.0f;

    const auto& rotation = glm::rotation(vec3(0, 0, 1), normal);

    vec3 v0 = center + radius * glm::rotate(rotation, vec3(cos(t), sin(t), 0));
    for (; t < glm::two_pi<float>(); t += dt)
    {
        vec3 v1 = center + radius * glm::rotate(rotation, vec3(cos(t + dt), sin(t + dt), 0));
        AddLine(category, v0, v1, color);
        v0 = v1;
    }
}

void DebugRenderer::AddSquare(DebugCategory::Enum category,
                              const glm::vec3& center,
                              float size,
                              const glm::vec3& normal,
                              const glm::vec4& color)
{
    if (!(m_categoryFlags & category)) return;

    const auto& rotation = glm::rotation(vec3(0, 0, 1), normal);

    const float s = size * 0.5f;
    auto A = center + glm::rotate(rotation, vec3(-s, -s, 0.0f));
    auto B = center + glm::rotate(rotation, vec3(-s, s, 0.0f));
    auto C = center + glm::rotate(rotation, vec3(s, s, 0.0f));
    auto D = center + glm::rotate(rotation, vec3(s, -s, 0.0f));

    // TODO: use normal

    AddLine(category, A, B, color);
    AddLine(category, B, C, color);
    AddLine(category, C, D, color);
    AddLine(category, D, A, color);
}

void DebugRenderer::AddCylinder(DebugCategory::Enum category,
                                const glm::vec3& center1,
                                const glm::vec3& center2,
                                float radius,
                                const vec4& color)
{
    if (!(m_categoryFlags & category)) return;

    constexpr float dt = glm::two_pi<float>() / 16.0f;
    float t = 0.0f;

    const auto& diff = center2 - center1;
    const auto& rotation = glm::rotation(vec3(0, 0, 1), glm::normalize(diff));

    vec3 v0 = center1 + radius * glm::rotate(rotation, vec3(cos(t), sin(t), 0));
    for (; t < glm::two_pi<float>(); t += dt)
    {
        vec3 v1 = center1 + radius * glm::rotate(rotation, vec3(cos(t + dt), sin(t + dt), 0));
        AddLine(category, v0, v1, color);
        AddLine(category, v0 + diff, v1 + diff, color);
        AddLine(category, v0, v0 + diff, color);
        v0 = v1;
    }
}
