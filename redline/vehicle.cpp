#include "vehicle.hpp"

#include <imgui/imgui.h>
#include <magic_enum/magic_enum.hpp>

#include "imgui_spline_helper.hpp"
#include "core/ecs.hpp"
#include "core/resources.hpp"
#include "core/transform.hpp"
#include "rendering/model.hpp"

BuickGrandNational87::BuickGrandNational87()
{
    // Render Car
    const auto body = CreateCarBody();
    // Positions: X=right, Y=forward, Z=up
    CreateCarWheel(body, "FL", { 0.800f,  1.350f, 0.120f}, false);
    CreateCarWheel(body, "FR", {-0.800f,  1.350f, 0.120f}, true);
    CreateCarWheel(body, "RL", { 0.800f, -1.350f, 0.120f}, false);
    CreateCarWheel(body, "RR", {-0.800f, -1.350f, 0.120f}, true);
}

/// Helper: create a quaternion from Euler angles in degrees (XYZ order)
inline glm::quat EulerDeg(float x, float y, float z)
{
    return glm::quat(glm::radians(glm::vec3(x, y, z)));
}

bee::Entity BuickGrandNational87::CreateCarBody()
{
    const auto entity = bee::Engine.ECS().CreateEntity();
    bee::Engine.ECS().CreateComponent<Vehicle>(entity);
    auto& transform = bee::Engine.ECS().CreateComponent<bee::Transform>(entity);
    transform.Name = "Buick_Grand_National_87";
    // Vehicle entity has identity rotation: forward = +Y, up = +Z

    // Child pivot to convert glTF Y-up to game Z-up
    const auto pivot = bee::Engine.ECS().CreateEntity();
    auto& pivotTransform = bee::Engine.ECS().CreateComponent<bee::Transform>(pivot);
    pivotTransform.Name = "ModelPivot";
    pivotTransform.SetRotation(EulerDeg(90.0f, 0.0f, 0.0f));
    pivotTransform.SetParent(entity);

    const auto model = bee::Engine.Resources().Load<bee::Model>(
        bee::FileIO::Directory::Assets,
        "vehicles/buick_grand_national_87/Car_Buick_GrandNational_1987.glb"
    );
    model->Instantiate(pivot);

    return entity;
}

void BuickGrandNational87::CreateCarWheel(const bee::Entity parent, const std::string& affix, const glm::vec3& position, const bool mirror)
{
    const auto entity = bee::Engine.ECS().CreateEntity();
    auto& transform = bee::Engine.ECS().CreateComponent<bee::Transform>(entity);
    transform.Name = "Buick_Grand_National_87_Wheel_" + affix;
    transform.SetTranslation(position);
    transform.SetParent(parent);

    // Child pivot: convert glTF Y-up to game Z-up, and mirror if needed
    const auto pivot = bee::Engine.ECS().CreateEntity();
    auto& pivotTransform = bee::Engine.ECS().CreateComponent<bee::Transform>(pivot);
    pivotTransform.Name = "WheelPivot";
    pivotTransform.SetRotation(EulerDeg(90.0f, 0.0f, mirror ? 180.0f : 0.0f));
    pivotTransform.SetParent(entity);

    const auto model = bee::Engine.Resources().Load<bee::Model>(
        bee::FileIO::Directory::Assets,
        "vehicles/buick_grand_national_87/Car_Buick_GrandNational_1987_Wheel.glb"
    );
    model->Instantiate(pivot);
}

VehicleSystem::VehicleSystem()
{
}

void VehicleSystem::Update(float dt)
{
    System::Update(dt);
}

void VehicleSystem::OnPanel()
{
    static float topSpeed = 70.0f;

    ImGui::SliderFloat("Max Speed (m/s)", &topSpeed, 0.0f, 100.0f);
    ImGui::SliderFloat("Engine Force", &engineForce, 0.0f, 3500.0f);
    ImGui::SliderFloat("Drag", &drag, 0.0f, 1.0f);
    ImGui::SliderFloat("Roll Resistance", &rr, 0.0f, 50.0f);
    
    if (ImGui::CollapsingHeader("Straight Line Physics", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Traction
        bee::Engine.ECS().Registry.view<bee::Transform, Vehicle>().each(
            [&](bee::Transform& transform)
            {
                direction = transform.GetRotation() * glm::vec3{0, 1, 0};
            }
        );
        
        // Graph dimensions
        const float graphWidth = 450.0f;
        const float graphHeight = 280.0f;

        const ImU32 graphColors[6] = {
            IM_COL32(255, 80, 80, 255),   // 1st - Red
            IM_COL32(255, 180, 80, 255),  // 2nd - Orange
            IM_COL32(255, 255, 80, 255),  // 3rd - Yellow
            IM_COL32(80, 255, 80, 255),   // 4th - Green
            IM_COL32(80, 180, 255, 255),  // 5th - Blue
            IM_COL32(180, 80, 255, 255)   // 6th - Purple
        };

        static SplineCanvas canvas(graphWidth, graphHeight);
        canvas.SetBackgroundColor(IM_COL32(20, 22, 28, 255));

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 40.0f);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f);
        canvas.Begin();

        // Precompute force values per speed step
        
        float speedSection = topSpeed / 39.0f;
        float normalizedHeight = graphHeight;
        float graphSectionWidth = graphWidth / 39.0f;

        std::vector<float> engineValues(40);
        std::vector<float> dragValues(40);
        std::vector<float> rrValues(40);
        for (size_t i = 0; i < 40; ++i)
        {
            float s_ms = speedSection * static_cast<float>(i);
            engineValues[i] = engineForce; // TODO: replace with per-RPM data

            // Fdrag = -Cdrag * v * |v|, using direction as forward (0,1,0)
            glm::vec3 v = direction * s_ms;
            glm::vec3 fdrag = -drag * v * glm::length(v);
            dragValues[i] = glm::abs(fdrag.y);
            
            rrValues[i] = glm::abs((-rr * v).y);
        }

        // Auto-normalize: find max across all curves
        float maxValue = 1.0f;
        for (size_t i = 0; i < 40; ++i)
        {
            maxValue = glm::max(maxValue, engineValues[i]);
            maxValue = glm::max(maxValue, dragValues[i] + rrValues[i]);
        }

        float yMax = maxValue;
        canvas.DrawLabeledGrid({10, 5}, {0.0f, topSpeed}, {0.0f, yMax});

        {   /** Draw Engine Force */
            std::vector<glm::vec2> points(40);
            for (size_t i = 0; i < points.size(); ++i)
            {
                float y = graphHeight - (engineValues[i] / maxValue) * normalizedHeight;
                points[i] = glm::vec2(graphSectionWidth * static_cast<float>(i), y);
            }
            canvas.DrawSpline(points, graphColors[0]);
        }

        {   /** Draw Drag */
            std::vector<glm::vec2> points(40);
            for (size_t i = 0; i < points.size(); ++i)
            {
                float y = graphHeight - (dragValues[i] / maxValue) * normalizedHeight;
                points[i] = glm::vec2(graphSectionWidth * static_cast<float>(i), y);
            }
            canvas.DrawSpline(points, graphColors[2]);
        }
        
        {   /** Draw Roll Resistance */
            std::vector<glm::vec2> points(40);
            for (size_t i = 0; i < points.size(); ++i)
            {
                float y = graphHeight - (rrValues[i] / maxValue) * normalizedHeight;
                points[i] = glm::vec2(graphSectionWidth * static_cast<float>(i), y);
            }
            canvas.DrawSpline(points, graphColors[1]);
        }
        
        {   /** Draw Roll Resistance + Drag */
            std::vector<glm::vec2> points(40);
            for (size_t i = 0; i < points.size(); ++i)
            {
                float y = graphHeight - ((rrValues[i] + dragValues[i]) / maxValue) * normalizedHeight;
                points[i] = glm::vec2(graphSectionWidth * static_cast<float>(i), y);
            }
            canvas.DrawSpline(points, graphColors[3]);
        }
        
        canvas.End();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        // Legend
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Engine");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "RR");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.3f, 1.0f), "Drag");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "RR+Drag");
    }
}
