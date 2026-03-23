#include <string>

#include "core/ecs.hpp"
#include "core/engine.hpp"
#include "redline.hpp"
#include "vehicle.hpp"
#include "platform/opengl/device_gl.hpp"
#include "Systems/ChassisSystem.hpp"
#include "Systems/EngineSystem.hpp"
#include "Systems/GearboxSystem.hpp"
#include "Systems/InputSystem.hpp"
#include "Systems/SteeringSystem.hpp"
#include "Systems/WheelSystem.hpp"

using namespace bee;

int main(int, char**)
{
    Engine.Initialize();
    Engine.Device().SetWindowSize(1280, 720);
    Engine.ECS().CreateSystem<Redline>();
    Engine.ECS().CreateSystem<ChassisSystem>();
    Engine.ECS().CreateSystem<EngineSystem>();
    Engine.ECS().CreateSystem<GearboxSystem>();
    Engine.ECS().CreateSystem<InputSystem>();
    Engine.ECS().CreateSystem<SteeringSystem>();
    Engine.ECS().CreateSystem<WheelSystem>();
    Engine.Run();
    Engine.Shutdown();
}
