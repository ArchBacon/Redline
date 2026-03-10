#include "redline.hpp"

#include <glm/glm.hpp>
#include <imgui/imgui.h>

#include "Curve.h"
#include "vehicle.hpp"
#include "floor.hpp"
#include "core/engine.hpp"
#include "core/transform.hpp"
#include "platform/opengl/device_gl.hpp"
#include "platform/opengl/render_gl.hpp"
#include "tools/log.hpp"

Redline::Redline()
{
    auto& renderer = bee::Engine.ECS().CreateSystem<bee::Renderer>();
    renderer.LoadEnvironment(bee::FileIO::Directory::SharedAssets, "environments/Footprint_Court_2k.hdr");
    
    // Create Vehicle
    BuickGrandNational87();
    
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
    bee::Engine.ECS().Registry.view<bee::Transform, Vehicle>().each(
        [&](bee::Transform& transform, Vehicle& vehicle)
        {
            if (transform.Name == "Buick_Grand_National_87")
            {
                speed = vehicle.Speed();
            }
        }
    );
    
    {   // Camera Updates
        float fov = glm::clamp(glm::mix(70.f, 95.f, speed / 100.f * 3.6), 70.f, 95.f);
        // Y=forward offset for lookat target
        glm::vec3 lookatOffset = glm::clamp(glm::mix(glm::vec3(0.0f, 0.0f, 0.8f), glm::vec3(0.0f, 0.0f, 0.0f), speed / 100.f * 3.6), glm::vec3(0.0f, 0.0f, 0.8f), glm::vec3(0.0f, 0.0f, 0.0f));
        glm::vec3 positionOffset = glm::clamp(glm::mix(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.3f), speed / 100.f * 3.6), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.3f));

        [[maybe_unused]] auto& camTransform = bee::Engine.ECS().Registry.get<bee::Transform>(camera);
        auto& camProjection = bee::Engine.ECS().Registry.get<bee::Camera>(camera);
        camProjection.Projection = glm::perspective(glm::radians(fov), bee::Engine.Device().GetAspectRatio(), 0.2f, 500.0f);

        bee::Engine.ECS().Registry.view<bee::Transform, Vehicle>().each(
            [&](bee::Transform& vtransform, Vehicle&)
            {
                auto forward = vtransform.GetRotation() * glm::vec3{0, 1, 0};
                auto& position = vtransform.GetTranslation();
                auto offset = glm::vec3{0.0f, -3.1f, 1.40f};

                auto view = lookAt(position + offset + positionOffset, position + forward + lookatOffset, glm::vec3(0.0f, 0.0f, 1.0f));
                camTransform.SetFromMatrix(glm::inverse(view));
            }
        );
    }
}

void Redline::OnPanel()
{
    ImGui::SliderFloat("Speed", &speed, 0.0f, 100.0f / 3.6f);
}
