#include "WheelSystem.hpp"

#include <imgui/imgui.h>

#include "../Components/ChassisComponent.hpp"
#include "../Components/DriveInputComponent.hpp"
#include "../Components/EngineComponent.hpp"
#include "../Components/GearboxComponent.hpp"
#include "../Components/WheelComponent.hpp"
#include "core/engine.hpp"

void WheelSystem::Update(const float dt)
{
    bee::Engine.ECS().Registry.view<Wheel, const Engine, const Gearbox, const Chassis, const DriveInput>().each(
        [&](Wheel& wheel, const Engine& engine, const Gearbox& gearbox, const Chassis& chassis, const DriveInput& drive)
        {
            const float vLong = glm::dot(chassis.velocity, chassis.direction);
            const float gearRatio = gearbox.GetRatio(gearbox.activeGear);
            
            // ── Engine braking torque (off-throttle, in gear) ─────
            float engineBrakeTorque = 0.0f;
            if (drive.throttle == 0.0f && drive.brake == 0.0f && glm::abs(gearRatio) > 0.001f)
            {
                engineBrakeTorque = engine.engineBrakingTorque
                    * glm::abs(gearRatio)
                    * gearbox.diffRatio
                    * gearbox.efficiency
                ;
            }
            
            // ── Slip ratio ────────────────────────────────────────
            const float wheelSpeed = wheel.angularVelocity * wheel.radius;
            const float refSpeed = glm::max(glm::abs(vLong), glm::abs(wheelSpeed));
            const float denom = glm::max(refSpeed, 0.001f);
            wheel.slipRatio = (wheelSpeed - vLong) / denom;
            
            // ── Traction force = C_t * SR, clamped to grip limit ─
            const float gripLimit = wheel.mu * wheel.axleLoad;
            wheel.tractionForce = glm::clamp(wheel.C_traction * wheel.slipRatio, -gripLimit, gripLimit);

            const float reactionTorque = wheel.tractionForce * wheel.radius;
            const float netTorque = engine.driveTorque - reactionTorque - engineBrakeTorque;
            wheel.angularVelocity += (netTorque / wheel.inertia) * dt;
            
            // prevent driving backwards on engine brake torque
            if (engine.driveTorque == 0.0f && drive.brake == 0.0f)
            {
                wheel.angularVelocity = glm::max(wheel.angularVelocity, 0.0f);
            }
            
            // ── Braking / handbrake: lock wheels → SR = -1 → MaxTraction
            if (drive.brake > 0.0f || drive.handbrake > 0.0f)
            {
                wheel.angularVelocity = 0.0f;
            }
        }
    );
}

void WheelSystem::OnPanel()
{
    bee::Engine.ECS().Registry.view<const Wheel>().each([](const Wheel& wheel)
    {
        ImGui::Text("Wheel Spd  %.1f km/h", wheel.angularVelocity * wheel.radius * 3.6f);
        ImGui::Text("Ang Vel    %.2f rad/s", wheel.angularVelocity);
        ImGui::Text("Slip       %.3f", wheel.slipRatio);
        ImGui::Separator();
        ImGui::Text("Traction   %.0f N", wheel.tractionForce);
        ImGui::Text("Axle Load  %.0f N", wheel.axleLoad);
    });
}
