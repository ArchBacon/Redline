#include <string>

#include "core/ecs.hpp"
#include "core/engine.hpp"
#include "redline.hpp"
#include "platform/opengl/device_gl.hpp"

using namespace bee;

int main(int, char**)
{
    Engine.Initialize();
    Engine.Device().SetWindowSize(1280, 720);
    Engine.ECS().CreateSystem<Redline>();
    Engine.Run();
    Engine.Shutdown();
}
