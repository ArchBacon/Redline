#include "ChassisSystem.hpp"

#include <imgui/imgui.h>
#include <glm/glm.hpp>

#include "../Components/ChassisComponent.hpp"
#include "../Components/DriveInputComponent.hpp"
#include "../Components/WheelComponent.hpp"
#include "core/engine.hpp"
#include "core/transform.hpp"

ChassisSystem::ChassisSystem()
{
}

void ChassisSystem::Update(const float dt)
{
    bee::Engine.ECS().Registry.view<bee::Transform, Chassis, Wheel, const DriveInput>().each(
        [&](bee::Transform& transform, Chassis& chassis, Wheel& wheel, const DriveInput& drive)
        {
            const float speed = glm::length(chassis.velocity);
            
            // ── Weight transfer ───────────────────────────────────
            const float W = chassis.mass * 9.8f; // gravity
            chassis.W_front = (chassis.cgToRear / chassis.wheelbase) * W
                - (chassis.cgHeight / chassis.wheelbase) * chassis.mass * chassis.accelLong;
            chassis.W_rear  = (chassis.cgToFront / chassis.wheelbase) * W
                + (chassis.cgHeight / chassis.wheelbase) * chassis.mass * chassis.accelLong;
            
            wheel.axleLoad = chassis.W_rear; // RWD
            
            // ── Net longitudinal force ────────────────────────────
            const float C_braking = 10000.0f; // TODO: move to component
            const float3 F_traction = (drive.brake > 0.0f)
                ? -chassis.direction * (C_braking * drive.brake)
                : chassis.direction * wheel.tractionForce;
            
            const float3 F_drag = -chassis.C_drag * chassis.velocity * speed;
            const float3 F_rr = -(chassis.C_drag * 30.f) * chassis.velocity;
            const float3 F_net = F_traction + F_drag + F_rr;
            
            const float3 accel = F_net / chassis.mass;
            chassis.velocity += accel * dt;
            chassis.accelLong = glm::dot(accel, chassis.direction);
            
            // ── Stop guard ───────────────────────────────────────
            const float stopThreshold = 3.0f / 3.6f;
            if (drive.throttle == 0.0f && speed < stopThreshold)
            {
                const float blend = speed / stopThreshold;
                chassis.velocity *= blend;
                wheel.angularVelocity *= blend;
            }
            
            transform.SetTranslation(transform.GetTranslation() + chassis.velocity * dt);
        }
    );
}

void ChassisSystem::OnPanel()
{
    bee::Engine.ECS().Registry.view<const Chassis>().each([](const Chassis& chassis)
    {
        const float speed = glm::length(chassis.velocity);
        ImGui::Text("Speed      %.1f km/h  (%.2f m/s)", speed * 3.6f, speed);
        ImGui::Text("Accel      %.2f m/s²", chassis.accelLong);
        ImGui::Separator();
        ImGui::Text("W Front    %.0f N", chassis.W_front);
        ImGui::Text("W Rear     %.0f N", chassis.W_rear);
        ImGui::Separator();
        ImGui::Text("Dir        (%.2f, %.2f, %.2f)", chassis.direction.x, chassis.direction.y, chassis.direction.z);
    });
}
