#include "redline.hpp"

#include <glm/glm.hpp>
#include <imgui/imgui.h>

#include "Curve.h"
#include "vehicle.hpp"
#include "floor.hpp"
#include "Components/ChassisComponent.hpp"
#include "core/engine.hpp"
#include "core/transform.hpp"
#include "platform/opengl/device_gl.hpp"
#include "platform/opengl/render_gl.hpp"
#include "tools/log.hpp"
#include "Vehicles/BuickGrandNational87.hpp"

Redline::Redline()
{
    auto& renderer = bee::Engine.ECS().CreateSystem<bee::Renderer>();
    renderer.LoadEnvironment(bee::FileIO::Directory::SharedAssets, "environments/Footprint_Court_2k.hdr");
    
    // Create Vehicle
    Buick_GrandNational_87();
    
    // Create Camera
    {
        camera = bee::Engine.ECS().CreateEntity();
        auto& camTransform = bee::Engine.ECS().CreateComponent<bee::Transform>(camera);
        camTransform.Name = "Camera";
        bee::Engine.ECS().CreateComponent<bee::Camera>(camera);
    }
    
    // Create floor
    Floor(20000.0f, "greybox_grey_grid.png", 20000.0f * 0.8f);
}

void Redline::Update(float)
{
    bee::Engine.ECS().Registry.view<const bee::Transform, const PlayerCar, const Chassis>().each(
        [&](const bee::Transform& transform, const Chassis& chassis)
        {
            speed = glm::length(chassis.velocity);
            const float fov = glm::clamp(glm::mix(70.f, 95.f, speed / 100.f * 3.6), 70.f, 95.f);
            
            auto& camTransform = bee::Engine.ECS().Registry.get<bee::Transform>(camera);
            auto& camProj = bee::Engine.ECS().Registry.get<bee::Camera>(camera);
            
            camProj.Projection = glm::perspective(
                glm::radians(fov), 
                bee::Engine.Device().GetAspectRatio(), 
                0.2f, 
                500.0f
            );

            const float3 forward = chassis.direction;
            const float3 position = transform.GetTranslation();
            
            const auto view = glm::lookAt(
                position + glm::vec3(0.0f, -3.1f, 1.40f),
                position + forward,
                glm::vec3(0.0f, 0.0f, 1.0f)
            );
            camTransform.SetFromMatrix(glm::inverse(view));
        }
    );
}

void Redline::OnPanel()
{
    ImGui::SliderFloat("Speed", &speed, 0.0f, 100.0f / 3.6f);
}
