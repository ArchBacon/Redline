#include "redline.hpp"

#include <glm/glm.hpp>
#include <imgui/imgui.h>

#include "vehicle.hpp"
#include "core/engine.hpp"
#include "core/transform.hpp"
#include "platform/opengl/device_gl.hpp"
#include "platform/opengl/render_gl.hpp"

Redline::Redline()
{
    auto& renderer = bee::Engine.ECS().CreateSystem<bee::Renderer>();
    renderer.LoadEnvironment(bee::FileIO::Directory::SharedAssets, "environments/Footprint_Court_2k.hdr");
    
    // Create Vehicle
    BuickGrandNational87();
    
    // Create Camera
    camera = bee::Engine.ECS().CreateEntity();
    auto& camTransform = bee::Engine.ECS().CreateComponent<bee::Transform>(camera);
    camTransform.Name = "Camera";
    bee::Engine.ECS().CreateComponent<bee::Camera>(camera);
}

void Redline::Update(float)
{
    float fov = glm::mix(70.f, 110.f, speed / 100.f);
    glm::vec3 lookatOffset = glm::mix(glm::vec3(0.0f, 0.0f, 2.2f), glm::vec3(0.0f, 0.0f, 3.0f), speed / 100.f);

    auto& camTransform = bee::Engine.ECS().Registry.get<bee::Transform>(camera);
    auto& camProjection = bee::Engine.ECS().Registry.get<bee::Camera>(camera);
    camProjection.Projection = glm::perspective(glm::radians(fov), bee::Engine.Device().GetAspectRatio(), 0.2f, 500.0f);

    bee::Engine.ECS().Registry.view<bee::Transform, Vehicle>().each(
        [&](bee::Transform& vtransform)
        {
            auto forward = vtransform.GetRotation() * glm::vec3{0, 1, 0};
            auto& position = vtransform.GetTranslation();
            auto offset = glm::vec3{0.0f, 4.5f, 1.40f};

            auto view = lookAt(position + offset, -forward + lookatOffset, glm::vec3(0.0f, 0.0f, 1.0f));
            camTransform.SetFromMatrix(glm::inverse(view));
        }
    );
}

void Redline::OnPanel()
{
    ImGui::SliderFloat("Speed", &speed, 0.0f, 100.0f);
}
