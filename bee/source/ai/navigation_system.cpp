#include "ai/navigation_system.hpp"

#include "ai/navmesh.hpp"
#include "ai/navmesh_agent.hpp"
#include "core/engine.hpp"
#include "core/transform.hpp"
#include "physics/physics_components.hpp"
#include "rendering/debug_render.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

using namespace bee::ai;
using namespace glm;

NavigationSystem::NavigationSystem(float fixedDeltaTime, float agentRadius)
    : m_fixedDeltaTime(fixedDeltaTime), m_timeSinceLastFrame(0)
{
    // get all navmesh input
    geometry2d::PolygonList navmeshObstacles;
    geometry2d::PolygonList navmeshWalkableAreas;

    auto view = Engine.ECS().Registry.view<Transform, physics::PolygonCollider, ai::NavmeshElement>();
    for (const auto& [entity, transform, collider, nav] : view.each())
    {
        // get the polygon boundary vertices in world space
        vec2 pos(transform.GetTranslation());
        float angle = glm::eulerAngles(transform.GetRotation()).z;
        geometry2d::Polygon pts_world = collider.GetLocalPoints();
        for (auto& pt : pts_world) pt = pos + geometry2d::RotateCounterClockwise(pt, angle);

        if (nav.m_type == ai::NavmeshElement::Type::Obstacle)
            navmeshObstacles.push_back(pts_world);
        else
            navmeshWalkableAreas.push_back(pts_world);
    }

    // build the navmesh
    m_navmesh = std::make_unique<ai::Navmesh>(navmeshWalkableAreas, navmeshObstacles, agentRadius);
}

void NavigationSystem::Update(float dt)
{
    m_timeSinceLastFrame += dt;

    const auto& view = Engine.ECS().Registry.view<ai::NavmeshAgent, physics::Body>();

    if (m_timeSinceLastFrame >= m_fixedDeltaTime)
    {
        // handle navmesh agent control
        for (const auto& [entity, agent, body] : view.each())
        {
            // recompute path?
            if (agent.ShouldRecomputePath()) agent.ComputePath(*m_navmesh, body.GetPosition());

            // update velocity
            agent.ComputePreferredVelocity(body.GetPosition(), m_fixedDeltaTime);
        }

        m_timeSinceLastFrame -= m_fixedDeltaTime;
    }

    // link agents to physics
    for (const auto& [entity, agent, body] : view.each())
    {
        body.SetLinearVelocity(agent.GetPreferredVelocity());
    }

    // debug drawing for the navmesh
    if ((Engine.DebugRenderer().GetCategoryFlags() & DebugCategory::AINavigation) != 0)
    {
        // debug drawing for navmesh polygons
        glm::vec4 color_navmesh(0.0f, 0.6f, 1.0f, 1.0f);
        for (const auto& polygon : m_navmesh->GetPolygons())
        {
            size_t n = polygon.size();
            for (size_t i = 0; i < n; ++i)
            {
                const auto& p1 = polygon[i];
                const auto& p2 = polygon[(i + 1) % n];
                Engine.DebugRenderer().AddLine(DebugCategory::AINavigation,
                                               glm::vec3(p1, 0.1f),
                                               glm::vec3(p2, 0.1f),
                                               color_navmesh);
            }
        }

        // debug drawing for navmesh graph
        const auto& graph = m_navmesh->GetGraph();

        glm::vec4 color_graph(0.7f, 0.0f, 0.1f, 1.0f);
        for (size_t v = 0; v < graph.GetNumberOfVertices(); ++v)
        {
            const auto& p1 = graph.GetVertex((int)v).position;
            Engine.DebugRenderer().AddCircle(DebugCategory::AINavigation,
                                             glm::vec3(p1, 0.1f),
                                             0.2f,
                                             glm::vec3(0, 0, 1),
                                             color_graph);

            for (const auto& edge : graph.GetEdgesFromVertex((int)v))
            {
                const auto& p2 = graph.GetVertex(edge.m_targetVertex).position;
                Engine.DebugRenderer().AddLine(DebugCategory::AINavigation,
                                               glm::vec3(p1, 0.1f),
                                               glm::vec3(p2, 0.1f),
                                               color_graph);
            }
        }
    }

    // debug drawing for agent paths
    if ((Engine.DebugRenderer().GetCategoryFlags() & DebugCategory::AIDecision) != 0)
    {
        vec4 color_path(1.0f, 1.0f, 0.0f, 1.0f);
        for (const auto& [entity, agent, body] : view.each())
        {
            const auto& path = agent.GetPath();
            if (!path.empty())
            {
                for (size_t p = 0; p < path.size() - 1; ++p)
                {
                    const auto& p1 = path[p];
                    const auto& p2 = path[p + 1];
                    Engine.DebugRenderer().AddLine(DebugCategory::AIDecision, vec3(p1, 0.15f), vec3(p2, 0.15f), color_path);
                    Engine.DebugRenderer().AddCircle(DebugCategory::AIDecision,
                                                     vec3(p1, 0.15f),
                                                     0.15f,
                                                     vec3(0, 0, 1),
                                                     color_path);
                }
                Engine.DebugRenderer().AddCircle(DebugCategory::AIDecision,
                                                 vec3(path.back(), 0.15f),
                                                 0.3f,
                                                 vec3(0, 0, 1),
                                                 color_path);
            }
        }
    }
}
