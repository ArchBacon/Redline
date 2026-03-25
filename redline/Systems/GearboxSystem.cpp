#include "GearboxSystem.hpp"

#include <cstdio>
#include <imgui/imgui.h>

#include "../Components/ChassisComponent.hpp"
#include "../Components/DriveInputComponent.hpp"
#include "../Components/EngineComponent.hpp"
#include "../Components/GearboxComponent.hpp"
#include "../Components/WheelComponent.hpp"
#include "core/engine.hpp"

void GearboxSystem::Update(const float)
{
    bee::Engine.ECS().Registry
        .view<Gearbox, Wheel, const Engine, const Chassis, const DriveInput>()
        .each([&](Gearbox& gearbox, Wheel& wheel, const Engine& engine, const Chassis& chassis, const DriveInput& drive)
        {
            const float vLong = glm::dot(chassis.velocity, chassis.direction);

            if (drive.throttle > 0.0f && gearbox.activeGear <= 0 && vLong > -0.5f)
            {
                gearbox.activeGear = 1;
                wheel.angularVelocity = glm::max(wheel.angularVelocity, 0.0f);
            }
            else if (drive.brake > 0.0f && gearbox.activeGear >= 0 && vLong < 0.5f)
            {
                gearbox.activeGear = -1; // reverse
                wheel.angularVelocity = glm::min(wheel.angularVelocity, 0.0f);
            }
            
            // Auto shift
            if (gearbox.activeGear > 0)
            {
                const float upRPM = engine.torqueCurve.GetMaxT() * 0.92f;
                const float downRPM = engine.torqueCurve.GetMaxT() * 0.45f;

                if (engine.currentRPM >= upRPM && gearbox.activeGear < gearbox.NumForwardGears()) gearbox.activeGear = gearbox.activeGear++;
                else if (engine.currentRPM <= downRPM && gearbox.activeGear > 1) gearbox.activeGear = gearbox.activeGear--;
            }
        });
}

void GearboxSystem::OnPanel()
{
    bee::Engine.ECS().Registry.view<const Gearbox>().each([](const Gearbox& gearbox)
    {
        char gearBuf[4] = "N";
        if (gearbox.activeGear < 0) gearBuf[0] = 'R', gearBuf[1] = '\0';
        else if (gearbox.activeGear > 0) std::snprintf(gearBuf, sizeof(gearBuf), "%d", (int)gearbox.activeGear);

        ImGui::Text("Gear       %s", gearBuf);
        ImGui::Text("Ratio      %.3f", gearbox.GetRatio(gearbox.activeGear));
        ImGui::Separator();
        ImGui::Text("Diff       %.2f", gearbox.diffRatio);
        ImGui::Text("Efficiency %.0f%%", gearbox.efficiency * 100.0f);
    });
}
