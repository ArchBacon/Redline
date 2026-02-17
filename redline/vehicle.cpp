#include "vehicle.hpp"

#include "core/ecs.hpp"
#include "core/resources.hpp"
#include "core/transform.hpp"
#include "rendering/model.hpp"

BuickGrandNational87::BuickGrandNational87()
{
    // Render Car
    const auto body = CreateCarBody();
    CreateCarWheel(body, "FL", { 0.800f, 0.120f,  1.350f});
    CreateCarWheel(body, "FR", {-0.800f, 0.120f,  1.350f});
    CreateCarWheel(body, "RL", { 0.800f, 0.120f, -1.350f});
    CreateCarWheel(body, "RR", {-0.800f, 0.120f, -1.350f});
}

bee::Entity BuickGrandNational87::CreateCarBody()
{
    const auto entity = bee::Engine.ECS().CreateEntity();
    bee::Engine.ECS().CreateComponent<Vehicle>(entity);
    auto& transform = bee::Engine.ECS().CreateComponent<bee::Transform>(entity);
    transform.Name = "Buick_Grand_National_87";
    transform.SetRotation(glm::angleAxis(glm::half_pi<float>(), glm::vec3(1, 0, 0)));

    const auto model = bee::Engine.Resources().Load<bee::Model>(
        bee::FileIO::Directory::Assets,
        "vehicles/buick_grand_national_87/Car_Buick_GrandNational_1987.glb"
    );
    model->Instantiate(entity);
    
    return entity;
}

void BuickGrandNational87::CreateCarWheel(const bee::Entity parent, const std::string& affix, const glm::vec3& position)
{
    const auto entity = bee::Engine.ECS().CreateEntity();
    auto& transform = bee::Engine.ECS().CreateComponent<bee::Transform>(entity);
    transform.Name = "Buick_Grand_National_87_Wheel_" + affix;
    transform.SetRotation(glm::angleAxis(glm::half_pi<float>(), glm::vec3(1, 0, 0)));
    transform.SetTranslation(position);

    const auto model = bee::Engine.Resources().Load<bee::Model>(
        bee::FileIO::Directory::Assets,
        "vehicles/buick_grand_national_87/Car_Buick_GrandNational_1987_Wheel.glb"
    );
    model->Instantiate(entity);
    transform.SetParent(parent);
}
