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
            bee::Log::Info(std::to_string(vehicle.RPM()).c_str());
        }
    );
}

void VehicleSystem::Update(float dt)
{
    const float wKey = bee::Engine.Input().GetKeyboardKey(bee::Input::KeyboardKey::W);
    const float sKey = bee::Engine.Input().GetKeyboardKey(bee::Input::KeyboardKey::S);
    const float handbrake = bee::Engine.Input().GetKeyboardKey(bee::Input::KeyboardKey::Space);

    bee::Engine.ECS().Registry.view<bee::Transform, Vehicle>().each(
        [&](bee::Transform& transform, Vehicle& vehicle)
        {
            const float vLong = glm::dot(vehicle.v, vehicle.u);

            // ── Input: map W/S to throttle/brake based on direction of travel ─────
            // W: accelerate forward, or brake if currently reversing
            // S: brake if going forward, or engage reverse and accelerate backward
            float throttle   = 0.0f;
            float brakeInput = 0.0f;

            if (wKey > 0.0f)
            {
                if (vLong < -0.5f)
                    brakeInput = wKey;           // moving backward: brake first
                else
                {
                    if (vehicle.activeGear <= 0) // shift out of neutral/reverse
                    {
                        vehicle.activeGear = 1;
                        vehicle.wheelAngularVelocity = glm::max(vehicle.wheelAngularVelocity, 0.0f);
                    }
                    throttle = wKey;
                }
            }

            if (sKey > 0.0f)
            {
                if (vLong > 0.5f)
                    brakeInput = sKey;           // moving forward: brake first
                else
                {
                    if (vehicle.activeGear >= 0) // shift into reverse
                    {
                        vehicle.activeGear = -1;
                        vehicle.wheelAngularVelocity = glm::min(vehicle.wheelAngularVelocity, 0.0f);
                    }
                    throttle = sKey;
                }
            }

            // ── Auto gear shifting (forward gears only) ───────────────────────────
            if (vehicle.activeGear > 0)
            {
                const float preShiftRatio = vehicle.GetGearRatio(vehicle.activeGear);
                const float preShiftRPM = glm::abs(vehicle.wheelAngularVelocity) * glm::abs(preShiftRatio)
                                          * vehicle.differentialRatio * 60.0f / glm::two_pi<float>();

                const float upshiftRPM   = vehicle.torqueCurve->GetMaxT() * 0.92f;
                const float downshiftRPM = vehicle.torqueCurve->GetMaxT() * 0.35f;

                if (preShiftRPM >= upshiftRPM && vehicle.activeGear < vehicle.gears)
                    vehicle.activeGear++;
                else if (preShiftRPM <= downshiftRPM && vehicle.activeGear > 1)
                    vehicle.activeGear--;
            }

            // ── RPM (recomputed after potential gear shift) ───────────────────────
            const float gearRatio = vehicle.GetGearRatio(vehicle.activeGear);
            const float rawRPM = (glm::abs(gearRatio) > 0.001f)
                ? glm::abs(vehicle.wheelAngularVelocity) * glm::abs(gearRatio)
                  * vehicle.differentialRatio * 60.0f / glm::two_pi<float>()
                : 0.0f;
            const float rpm = glm::clamp(rawRPM,
                vehicle.torqueCurve->GetMinT(), vehicle.torqueCurve->GetMaxT());

            // ── Drive torque + rev limiter ────────────────────────────────────────
            float driveTorque = 0.0f;
            if (throttle > 0.0f && glm::abs(gearRatio) > 0.001f)
            {
                if (rawRPM <= vehicle.torqueCurve->GetMaxT())
                {
                    // Powered: T_drive = throttle · T_engine(rpm) · x_g · x_d · η
                    driveTorque = throttle * vehicle.Torque(rpm)
                                  * gearRatio * vehicle.differentialRatio * vehicle.transmissionEfficiency;
                }
                else
                {
                    // Rev limiter: clamp ω to redline equivalent
                    const float maxOmega = vehicle.torqueCurve->GetMaxT() * glm::two_pi<float>()
                                           / (60.0f * glm::abs(gearRatio) * vehicle.differentialRatio);
                    vehicle.wheelAngularVelocity = glm::sign(vehicle.wheelAngularVelocity) * maxOmega;
                }
            }

            // ── Engine braking (off-throttle, no brake pedal, in gear) ────────────
            // Directly set ω to a mild negative slip rather than integrating a huge
            // torque — avoids numerical oscillation that would cancel deceleration.
            // The slip ratio then produces a gentle braking traction force on the car.
            if (throttle == 0.0f && brakeInput == 0.0f && glm::abs(gearRatio) > 0.001f)
                vehicle.wheelAngularVelocity = (vLong / vehicle.wheelRadius) * (1.0f - vehicle.engineBrakingSlip);

            // ── Traction force from slip ratio ────────────────────────────────────
            const float tractionForce = vehicle.TractionForceScalar();

            // ── Net torque on drive axle (throttle path only) ─────────────────────
            const float netTorque = driveTorque - tractionForce * vehicle.wheelRadius;
            if (throttle > 0.0f)
                vehicle.wheelAngularVelocity += (netTorque / vehicle.wheelInertia) * dt;

            // ── Manual braking / handbrake: lock wheels → SR = -1 → MaxTraction ───
            if (brakeInput > 0.0f || handbrake > 0.0f)
                vehicle.wheelAngularVelocity = 0.0f;

            // ── Net force on car body ─────────────────────────────────────────────
            const glm::vec3 fNet = vehicle.u * tractionForce
                                 + vehicle.Drag()
                                 + vehicle.RollingResistance();

            vehicle.SetVelocity(vehicle.v + (fNet / vehicle.M) * dt);

            // Ramp to zero at low speed when not accelerating.
            constexpr float stopThreshold = 3.0f / 3.6f; // 3 km/h in m/s
            if (throttle == 0.0f && vehicle.Speed() < stopThreshold)
            {
                const float blend = vehicle.Speed() / stopThreshold; // 1 at threshold, 0 at rest
                vehicle.SetVelocity(vehicle.v * blend);
                vehicle.wheelAngularVelocity *= blend;
            }

            transform.SetTranslation(transform.GetTranslation() + vehicle.v * dt);
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

            if (vehicle.activeGear < 0)
                ImGui::Text("Gear: R  |  RPM: %.0f  |  SR: %.3f", vehicle.RPM(), vehicle.SlipRatio());
            else if (vehicle.activeGear == 0)
                ImGui::Text("Gear: N  |  RPM: %.0f  |  SR: %.3f", vehicle.RPM(), vehicle.SlipRatio());
            else
                ImGui::Text("Gear: %d  |  RPM: %.0f  |  SR: %.3f", vehicle.activeGear, vehicle.RPM(), vehicle.SlipRatio());

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

            if (ImGui::CollapsingHeader("Slip Curve", ImGuiTreeNodeFlags_DefaultOpen))
            {
                static SplineCanvas canvas(graphWidth, graphHeight);
                canvas.SetBackgroundColor(IM_COL32(20, 22, 28, 255));

                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 40.0f);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f);
                canvas.Begin();

                const float srMin     = -1.2f;
                const float srMax     =  1.2f;
                const float maxForce  = vehicle.MaxTraction();
                const float peakSlip  = maxForce / vehicle.tractionConstant;

                // Helper: map slip ratio → canvas X, force → canvas Y (0 force = vertical centre)
                auto toX = [&](float sr)    { return (sr - srMin) / (srMax - srMin) * graphWidth; };
                auto toY = [&](float force) { return graphHeight * 0.5f - (force / maxForce) * graphHeight * 0.5f; };

                canvas.DrawLabeledGrid({12, 8}, {srMin, srMax}, {-maxForce, maxForce}, "Slip Ratio", "Traction (N)");

                // Zero axes
                canvas.DrawLine({toX(0.f), 0.f}, {toX(0.f), graphHeight}, IM_COL32(60, 64, 72, 255));
                canvas.DrawLine({0.f, toY(0.f)}, {graphWidth, toY(0.f)}, IM_COL32(60, 64, 72, 255));

                // Slip curve: F = C_t * SR, clamped to ±MaxTraction
                // Use DrawPolyline (not spline) so the kink at the clamp points stays sharp.
                {
                    std::vector<glm::vec2> pts;
                    pts.reserve(44);
                    for (int i = 0; i <= 40; ++i)
                    {
                        float sr    = srMin + (srMax - srMin) * static_cast<float>(i) / 40.f;
                        float force = glm::clamp(vehicle.tractionConstant * sr, -maxForce, maxForce);
                        pts.push_back({toX(sr), toY(force)});
                    }
                    canvas.DrawPolyline(pts, graphColors[0], 2.0f);
                }

                // Peak slip markers (where the curve knee is)
                canvas.DrawLine({toX( peakSlip), 0.f}, {toX( peakSlip), graphHeight}, IM_COL32(255, 200, 60, 80));
                canvas.DrawLine({toX(-peakSlip), 0.f}, {toX(-peakSlip), graphHeight}, IM_COL32(255, 200, 60, 80));

                // Current operating point
                const float currentSR    = glm::clamp(vehicle.SlipRatio(), srMin, srMax);
                const float currentForce = glm::clamp(vehicle.tractionConstant * currentSR, -maxForce, maxForce);
                canvas.DrawLine({toX(currentSR), 0.f}, {toX(currentSR), graphHeight}, IM_COL32(80, 220, 255, 180));
                canvas.DrawDot({toX(currentSR), toY(currentForce)}, 5.f, IM_COL32(80, 220, 255, 255), IM_COL32(255, 255, 255, 200));

                canvas.End();
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Spacing();

                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Curve");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.78f, 0.24f, 1.0f), "Peak slip (%.2f)", peakSlip);
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.31f, 0.86f, 1.0f, 1.0f), "Current SR (%.3f)", vehicle.SlipRatio());
            }
        }
    );
}
