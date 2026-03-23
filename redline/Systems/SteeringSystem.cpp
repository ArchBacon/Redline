#include "SteeringSystem.hpp"

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "../Components/ChassisComponent.hpp"
#include "../Components/DriveInputComponent.hpp"
#include "../Components/SteeringComponent.hpp"
#include "core/engine.hpp"
#include "core/transform.hpp"

void SteeringSystem::Update(const float dt)
{
    bee::Engine.ECS().Registry.view<bee::Transform, Steering, Chassis, const DriveInput>()
        .each([&](bee::Transform& transform, Steering& steering, Chassis& chassis, const DriveInput& drive)
        {
            steering.currentInput = -drive.steer;
            steering.currentAngle = steering.maxAngleRad * -drive.steer;
            
            const float speed = glm::length(chassis.velocity);
            if (speed < 0.1f) return;
            
            const float turnRadius = (glm::abs(steering.currentAngle) > 0.001f)
                ? chassis.wheelbase / glm::sin(glm::abs(steering.currentAngle))
                : std::numeric_limits<float>::infinity();
            
            steering.yawRate = (glm::abs(steering.currentAngle) > 0.001f)
                ? speed/ turnRadius * glm::sign(steering.currentAngle)
                : 0.0f;

            const glm::mat3 rot = glm::mat3(glm::rotate(glm::mat4(1.0f), steering.yawRate * dt, glm::vec3(0.0f, 0.0f, 1.0f)));
            chassis.direction = glm::normalize(rot * chassis.direction);

            chassis.velocity = chassis.direction * speed;
            const float angle = -glm::atan(chassis.direction.x, chassis.direction.y);
            transform.SetRotation(glm::toQuat(glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f))));
        }
    );
}

void SteeringSystem::OnPanel()
{
    bee::Engine.ECS().Registry.view<const Steering>().each([](const Steering& steering)
    {
        ImGui::Text("Angle      %.1f° / %.1f°", glm::degrees(steering.currentAngle), glm::degrees(steering.maxAngleRad));
        ImGui::ProgressBar((steering.currentAngle / steering.maxAngleRad) * 0.5f + 0.5f, ImVec2(-1, 0), "");
        ImGui::Separator();
        ImGui::Text("Yaw Rate   %.3f rad/s", steering.yawRate);
    });
}
