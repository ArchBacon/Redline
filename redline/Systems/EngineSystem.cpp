#include "EngineSystem.hpp"

#include <imgui/imgui.h>
#include <glm/glm.hpp>

#include "../Components/DriveInputComponent.hpp"
#include "../Components/EngineComponent.hpp"
#include "../Components/GearboxComponent.hpp"
#include "../Components/WheelComponent.hpp"
#include "core/engine.hpp"
#include "tools/log.hpp"

void EngineSystem::Update(const float)
{
    bee::Engine.ECS().Registry.view<Engine, const Gearbox, const Wheel, const DriveInput>().each(
        [&](Engine& engine, const Gearbox& gearbox, const Wheel& wheel, const DriveInput& drive)
        {
            const float gearRatio = gearbox.GetRatio(gearbox.activeGear);
            
            // Derive RPM from wheel angular velocity
            const float RPM = (glm::abs(gearRatio) > 0.001f)
                ? glm::abs(wheel.angularVelocity) * glm::abs(gearRatio) * gearbox.diffRatio * 60.0f / glm::two_pi<float>()
                : 0.0f;
            
            bee::Log::Info("EngineSystem::Update eng vel: {}", wheel.angularVelocity);
            bee::Log::Info("EngineSystem::Update RPM: {}", RPM);
            
            engine.currentRPM = glm::clamp(
                RPM,
                engine.torqueCurve.GetMinT(),
                engine.torqueCurve.GetMaxT()
            );
            
            bee::Log::Info("EngineSystem::Update currentRPM: {}", engine.currentRPM);
            
            engine.driveTorque = 0.0f;                                   // rev limiter
            if (drive.throttle > 0.0f && glm::abs(gearRatio) > 0.001f && RPM <= engine.torqueCurve.GetMaxT())
            {
                engine.driveTorque = drive.throttle
                    * engine.torqueCurve.GetValueAt(engine.currentRPM)
                    * gearbox.diffRatio
                    * gearbox.diffRatio
                    * gearbox.efficiency
                ;
            }
        }
    );
}

void EngineSystem::OnPanel()
{
    bee::Engine.ECS().Registry.view<const Engine>().each([](const Engine& engine)
    {
        const float maxRPM = engine.torqueCurve.GetMaxT();
        ImGui::Text("RPM        %.0f / %.0f", engine.currentRPM, maxRPM);
        ImGui::ProgressBar(engine.currentRPM / maxRPM, ImVec2(-1, 0), "");
        ImGui::Separator();
        ImGui::Text("Drive Trq  %.1f Nm", engine.driveTorque);
        ImGui::Text("Brake Trq  %.1f Nm", engine.engineBrakingTorque);
    });
}
