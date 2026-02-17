#include "redline.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

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
    const auto entity = bee::Engine.ECS().CreateEntity();
    auto& transform = bee::Engine.ECS().CreateComponent<bee::Transform>(entity);
    transform.Name = "Camera";
    auto projection = glm::perspective(glm::radians(70.0f), bee::Engine.Device().GetAspectRatio(), 0.2f, 500.0f);
    auto view = lookAt(glm::vec3(5.0f, 5.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    transform.SetFromMatrix(glm::inverse(view));
    bee::Engine.ECS().CreateComponent<bee::Camera>(entity).Projection = projection;
    
    bee::Engine.ECS().Registry.view<bee::Transform, Vehicle>().each(
        [this, &entity](bee::Transform& vtransform)
        {
            auto& camTransform = bee::Engine.ECS().Registry.get<bee::Transform>(entity);
            auto forward = vtransform.GetRotation() * glm::vec3{0, 1, 0};
            auto& position = vtransform.GetTranslation();
            auto offset = glm::vec3{0.0f, 4.5f, 1.40f};

            auto view = lookAt(position + offset, -forward + glm::vec3(0.0f, 0.0f, 2.2f), glm::vec3(0.0f, 0.0f, 1.0f));
            camTransform.SetFromMatrix(glm::inverse(view));
        }
    );
}

void Redline::Update(float) {}