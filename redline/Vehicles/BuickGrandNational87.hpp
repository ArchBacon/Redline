#pragma once

#include "../Components/ChassisComponent.hpp"
#include "../Components/DriveInputComponent.hpp"
#include "../Components/EngineComponent.hpp"
#include "../Components/GearboxComponent.hpp"
#include "../Components/SteeringComponent.hpp"
#include "../Components/WheelComponent.hpp"
#include "../Components/WheelVisualComponent.hpp"
#include "core/ecs.hpp"
#include "core/engine.hpp"
#include "core/resources.hpp"
#include "core/transform.hpp"
#include "rendering/model.hpp"

inline void CreateCarBody(const bee::Entity entity)
{
    const auto body = bee::Engine.ECS().CreateEntity();
    auto& transform = bee::Engine.ECS().CreateComponent<bee::Transform>(body);
    transform.Name = "Buick_Grand_National_87_Body";
    transform.SetRotation(glm::quat(glm::radians(float3(90.0f, 0.0f, 180.0f))));
    transform.SetTranslation({0.0f, 0.0f, 0.15f});
    transform.SetParent(entity);

    const auto model = bee::Engine.Resources().Load<bee::Model>(
        bee::FileIO::Directory::Assets,
        "vehicles/buick_grand_national_87/Car_Buick_GrandNational_1987.glb"
    );
    model->Instantiate(body);
}

inline void CreateCarWheel(const bee::Entity parent, const bee::Entity car, const std::string& affix, const glm::vec3& position, const bool mirror, const bool isFront)
{
    const auto entity = bee::Engine.ECS().CreateEntity();
    auto& transform = bee::Engine.ECS().CreateComponent<bee::Transform>(entity);
    transform.Name = "Buick_Grand_National_87_Wheel_" + affix;
    transform.SetTranslation(position + float3{0.0f, 0.0f, 0.15f});
    transform.SetParent(parent);

    auto& visual = bee::Engine.ECS().CreateComponent<WheelVisual>(entity);
    visual.car     = car;
    visual.isFront = isFront;
    visual.mirror  = mirror;

    const auto model = bee::Engine.Resources().Load<bee::Model>(
        bee::FileIO::Directory::Assets,
        "vehicles/buick_grand_national_87/Car_Buick_GrandNational_1987_Wheel.glb"
    );
    model->Instantiate(entity);
}

inline bee::Entity Buick_GrandNational_87()
{
    auto& ecs = bee::Engine.ECS();
    bee::Entity car = ecs.CreateEntity();
    
    // ── Transform ────────────────────────────────────────────
    auto& t = ecs.CreateComponent<bee::Transform>(car);
    t.Name = "Buick|_Grand_National_87";
    
    // ── Physics components ────────────────────────────────────
    auto& chassis = ecs.CreateComponent<Chassis>(car);
    chassis.mass      = 1530.0f;
    chassis.wheelbase = 2.746f;
    chassis.cgToFront = 2.746f * 0.57f;
    chassis.cgToRear  = 2.746f * 0.43f;
    chassis.cgHeight  = 1.387f;
    chassis.C_drag    = 0.38f;
    chassis.direction = {0.0f, 1.0f, 0.0f};
    
    auto& engine = ecs.CreateComponent<Engine>(car);
    engine.bmep = 2063016.0f;   // Pa — correct BMEP for GNX 3.8L turbo V6 (~625 Nm peak)
    engine.displacement = 3.8f;
    engine.cylinders = 6;
    engine.pumpingLossFraction = 0.15f;
    engine.torqueCurve = Curve("vehicles/buick_grand_national_87/Car_Buick_GrandNational_1987_TorqueData.csv");
    
    engine.Init();

    auto& gearbox = ecs.CreateComponent<Gearbox>(car);
    gearbox.gearRatios   = {2.74f, 1.57f, 1.00f, 0.67f}; 
    gearbox.reverseRatio = 4.5f;
    gearbox.diffRatio    = 3.42f;
    gearbox.efficiency   = 0.85f;
    
    // TODO: Make 4 wheels instead of 1
    auto& wheel = ecs.CreateComponent<Wheel>(car);
    wheel.radius = 0.33f;
    wheel.mass = 20.0f;
    wheel.mu = 1.0f;
    wheel.C_traction = 15000.0f;
    wheel.isDriven = true;
    wheel.Init();
    
    auto& steer = ecs.CreateComponent<Steering>(car);
    steer.maxAngleRad = glm::radians(14.0f);
    
    ecs.CreateComponent<DriveInput>(car);
    
    // ── Visual components ────────────────────────────────────
    CreateCarBody(car);
    CreateCarWheel(car, car, "FL", { 0.800f,  1.350f, 0.120f}, false, true);
    CreateCarWheel(car, car, "FR", {-0.800f,  1.350f, 0.120f}, true,  true);
    CreateCarWheel(car, car, "RL", { 0.800f, -1.350f, 0.120f}, false, false);
    CreateCarWheel(car, car, "RR", {-0.800f, -1.350f, 0.120f}, true,  false);
    
    ecs.CreateComponent<PlayerCar>(car);
    
    return car;
}
