#include <string>

#include "core/ecs.hpp"
#include "core/engine.hpp"
#include "redline.hpp"

using namespace bee;

int main(int, char**)
{
    Engine.Initialize();
    Engine.ECS().CreateSystem<Redline>();
    Engine.Run();
    Engine.Shutdown();
}
