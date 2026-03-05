#include "vehicle.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/implot.h>
#include <magic_enum/magic_enum.hpp>

#include "imgui_spline_helper.hpp"
#include "core/ecs.hpp"
#include "core/input.hpp"
#include "core/resources.hpp"
#include "core/transform.hpp"
#include "rendering/model.hpp"
#include "tools/log.hpp"

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
    VehicleData vehicleData {};
    
    const auto entity = bee::Engine.ECS().CreateEntity();
    bee::Engine.ECS().CreateComponent<Vehicle>(entity, vehicleData);
    auto& transform = bee::Engine.ECS().CreateComponent<bee::Transform>(entity);
    transform.Name = "Buick_Grand_National_87";
    // Vehicle entity has identity rotation: forward = +Y, up = +Z

    // Child pivot to convert glTF Y-up to game Z-up
    const auto pivot = bee::Engine.ECS().CreateEntity();
    auto& pivotTransform = bee::Engine.ECS().CreateComponent<bee::Transform>(pivot);
    pivotTransform.Name = "ModelPivot";
    pivotTransform.SetRotation(EulerDeg(90.0f, 0.0f, 180.0f));
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
    bee::Engine.ECS().Registry.view<bee::Transform, Vehicle>().each(
        [&](bee::Transform& transform, Vehicle& vehicle)
        {
            vehicle.u = transform.GetRotation() * glm::vec3{0, 1, 0}; // face forward
            bee::Log::Info(std::to_string(glm::abs(vehicle.DriveForce(3000, 1).y)));
        }
    );
}

void VehicleSystem::Update(float dt)
{
    const float accel = bee::Engine.Input().GetKeyboardKey(bee::Input::KeyboardKey::W);
    const float brake = bee::Engine.Input().GetKeyboardKey(bee::Input::KeyboardKey::S);
    
    bee::Engine.ECS().Registry.view<bee::Transform, Vehicle>().each(
        [&](bee::Transform& transform, Vehicle& vehicle)
        {
            if (accel > 0.0f)
                vehicle.SetVelocity(vehicle.Velocity() + dt * (vehicle.Acceleration(accel)));
            else
                vehicle.SetVelocity(vehicle.Velocity() + dt * (vehicle.Braking(brake)));
            
            transform.SetTranslation(transform.GetTranslation() + dt * vehicle.Velocity());
        }
    );
}

void VehicleSystem::OnPanel()
{
    bee::Engine.ECS().Registry.view<bee::Transform, Vehicle>().each(
        [](bee::Transform& transform, Vehicle& vehicle)
        {
            ImGui::Text("Top Speed  %.1f m/s  %.1f km/h", vehicle.TopSpeed(), vehicle.TopSpeed() * 3.6f);
            ImGui::Text("%s  %.2f m/s  %.2f km/h", transform.Name.c_str(), vehicle.Speed(), vehicle.Speed() * 3.6f);

            float topSpeed = 300.f / 3.6f; // Convert to m/s
            
            // Graph dimensions
            const float graphWidth = 450.0f;
            const float graphHeight = 280.0f;
        
            const ImU32 graphColors[6] = {
                IM_COL32(255, 80, 80, 255),   // 1st - Red
                IM_COL32(255, 180, 80, 255),  // 2nd - Orange
                IM_COL32(255, 255, 80, 255),  // 3rd - Yellow
                IM_COL32(80, 255, 80, 255),   // 4th - Green
                IM_COL32(80, 180, 255, 255),  // 5th - Blue
                IM_COL32(180, 80, 255, 255),  // 6th - Purple
            };
            
            if (ImGui::CollapsingHeader("Velocity/Force"))
            {
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
                    engineValues[i] = vehicle.engineForce;
            
                    // Fdrag = -Cdrag * v * |v|
                    glm::vec3 v = vehicle.Direction() * s_ms;
                    glm::vec3 fdrag = -vehicle.drag * v * glm::length(v);
                    dragValues[i] = glm::abs(fdrag.y);
                    
                    rrValues[i] = glm::abs((-vehicle.rr * v).y);
                }
            
                // Auto-normalize: find max across all curves
                float maxValue = 1.0f;
                for (size_t i = 0; i < 40; ++i)
                {
                    maxValue = glm::max(maxValue, engineValues[i]);
                    maxValue = glm::max(maxValue, dragValues[i] + rrValues[i]);
                }
            
                maxValue *= 1.1f;
                canvas.DrawLabeledGrid({10, 5}, {0.0f, topSpeed * 3.6f}, {0.0f, maxValue}, "Km/h", "Force");
            
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
                
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Spacing();
            }
            
            if (ImGui::CollapsingHeader("Torque/Power"))
            {
                static SplineCanvas canvas(graphWidth, graphHeight);
                canvas.SetBackgroundColor(IM_COL32(20, 22, 28, 255));
            
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 40.0f);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f);
                canvas.Begin();
                
                float normalizedHeight = graphHeight;
                float graphSectionWidth = graphWidth / 39.f;
            
                const float rpmMin = vehicle.torqueCurve->GetMinT();
                const float rpmMax = vehicle.torqueCurve->GetMaxT();
                std::vector<float> torqueValues(40);
                std::vector<float> horseValues(40);
                float step_size = (rpmMax - rpmMin) / 39.f;
                for (size_t i = 0; i < 40; ++i)
                {
                    float rpm = rpmMin + step_size * static_cast<float>(i);

                    torqueValues[i] = vehicle.Torque(rpm);
                    horseValues[i] = vehicle.Horsepower(rpm);
                }
            
                // Auto-normalize: find max across all curves
                float maxValue = 1.0f;
                for (size_t i = 0; i < 40; ++i)
                {
                    maxValue = glm::max(maxValue, torqueValues[i]);
                    maxValue = glm::max(maxValue, horseValues[i]);
                }
            
                maxValue *= 1.1f;
                canvas.DrawLabeledGrid({10, 5}, {rpmMin, rpmMax}, {0.0f, maxValue}, "RPM", "Torque / Power");
            
                {   /** Draw Torque */
                    std::vector<glm::vec2> points(40);
                    for (size_t i = 0; i < points.size(); ++i)
                    {
                        float y = graphHeight - (torqueValues[i] / maxValue) * normalizedHeight;
                        points[i] = glm::vec2(graphSectionWidth * static_cast<float>(i), y);
                    }
                    canvas.DrawSpline(points, graphColors[0]);
                }
                
                {   /** Draw Horsepower */
                    std::vector<glm::vec2> points(40);
                    for (size_t i = 0; i < points.size(); ++i)
                    {
                        float y = graphHeight - (horseValues[i] / maxValue) * normalizedHeight;
                        points[i] = glm::vec2(graphSectionWidth * static_cast<float>(i), y);
                    }
                    canvas.DrawSpline(points, graphColors[1]);
                }
                
                canvas.End();
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Spacing();
            
                // Legend
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Torque (N.m)");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "Power (kW)");
            }
            
            if (ImGui::CollapsingHeader("Torque Curve per Gear", ImGuiTreeNodeFlags_DefaultOpen))
            {
                static SplineCanvas canvas(graphWidth, graphHeight);
                canvas.SetBackgroundColor(IM_COL32(20, 22, 28, 255));
            
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 40.0f);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f);
                canvas.Begin();
                
                float normalizedHeight = graphHeight;

                const float rpmMin = vehicle.torqueCurve->GetMinT();
                const float rpmMax = vehicle.torqueCurve->GetMaxT();
                const float step_size = (rpmMax - rpmMin) / 39.f;

                // Y axis: wheel torque (peaks in 1st gear)
                float maxValue = 1.0f;
                for (size_t i = 0; i < 40; ++i)
                {
                    float rpm = rpmMin + step_size * static_cast<float>(i);
                    for (int g = 1; g <= static_cast<int>(vehicle.gears); ++g)
                        maxValue = glm::max(maxValue, vehicle.WheelTorque(rpm, g));
                }
                maxValue *= 1.1f;

                canvas.DrawLabeledGrid({10, 5}, {0.0f, rpmMax}, {0.0f, maxValue}, "Wheel RPM", "Wheel Torque (N.m)");

                for (int g = 1; g <= static_cast<int>(vehicle.gears); ++g)
                {
                    std::vector<glm::vec2> points(40);
                    for (size_t i = 0; i < 40; ++i)
                    {
                        float engineRPM  = rpmMin + step_size * static_cast<float>(i);
                        float wheelRPM   = vehicle.WheelRPM(engineRPM, g);
                        float wheelTorque = vehicle.WheelTorque(engineRPM, g);
                        float x = (wheelRPM / rpmMax) * graphWidth;
                        float y = graphHeight - (wheelTorque / maxValue) * normalizedHeight;
                        points[i] = glm::vec2(x, y);
                    }
                    canvas.DrawSpline(points, graphColors[g - 1]);
                }

                // Engine torque reference curve: engine RPM on same axis scale as wheel RPM
                {
                    std::vector<glm::vec2> enginePoints(40);
                    for (size_t i = 0; i < 40; ++i)
                    {
                        float engineRPM = rpmMin + step_size * static_cast<float>(i);
                        float x = (engineRPM / rpmMax) * graphWidth;
                        float y = graphHeight - (vehicle.Torque(engineRPM) / maxValue) * normalizedHeight;
                        enginePoints[i] = glm::vec2(x, y);
                    }
                    canvas.DrawSpline(enginePoints, graphColors[4]);
                }

                canvas.End();
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Spacing();

                // Legend
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "1st Gear");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "2nd Gear");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.3f, 1.0f), "3rd Gear");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "4th Gear");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.3f, 0.7f, 1.0f, 1.0f), "Engine");
            }
        }
    );
}
