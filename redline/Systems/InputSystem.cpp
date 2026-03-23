#include "InputSystem.hpp"

#include <imgui/imgui.h>

#include "../Components/DriveInputComponent.hpp"
#include "core/engine.hpp"
#include "core/input.hpp"

void InputSystem::Update(const float)
{
    const auto& input = bee::Engine.Input();
    
    const float accel = input.GetKeyboardKey(bee::Input::KeyboardKey::W);
    const float decel = input.GetKeyboardKey(bee::Input::KeyboardKey::S);
    const float steerLeft = input.GetKeyboardKey(bee::Input::KeyboardKey::A);
    const float steerRight = input.GetKeyboardKey(bee::Input::KeyboardKey::D);
    const float handbrake = input.GetKeyboardKey(bee::Input::KeyboardKey::Space);
    
    bee::Engine.ECS().Registry
        .view<DriveInput>()
        .each([&](DriveInput& drive)
        {
            drive.throttle = accel;
            drive.brake = decel;
            drive.handbrake = handbrake;
            drive.steer = steerRight - steerLeft;
        });
}

void InputSystem::OnPanel()
{
    bee::Engine.ECS().Registry.view<const DriveInput>().each([](const DriveInput& drive)
    {
        ImGui::Text("Throttle"); ImGui::SameLine(80); ImGui::ProgressBar(drive.throttle,    ImVec2(-1, 0), "");
        ImGui::Text("Brake");    ImGui::SameLine(80); ImGui::ProgressBar(drive.brake,       ImVec2(-1, 0), "");
        ImGui::Text("Handbrk"); ImGui::SameLine(80); ImGui::ProgressBar(drive.handbrake,   ImVec2(-1, 0), "");
        ImGui::Separator();
        ImGui::Text("Steer      %.2f", drive.steer);
    });
}
