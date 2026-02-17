#pragma once

#include <glm/vec2.hpp>
#include <optional>
#include "tools/inspectable.hpp"
#include "core/ecs.hpp"

namespace bee::physics
{
class Body;
struct CollisionData;
struct PolygonCollider;

/// <summary>
/// A struct that represents the result of a raycast query.
/// </summary>
struct RaycastResult
{
    /// The ID of the entity that the ray has hit.
    bee::Entity m_hitEntity{entt::null};
    /// The position at which the ray has first hit the boundary of the entity's collider.
    glm::vec2 m_hitPosition{0, 0};
    /// The normal vector at the ray's point of impact.
    glm::vec2 m_normal{0, 0};
    /// The distance that the ray has travelled until the point of impact.
    /// Can be used for comparing multiple raycast results.
    float m_rayDistance{0.f};

    /// Returns whether or not this RaycastResult represents a valid result.
    /// This is based on the rayDistance member variable.
    inline bool IsValid() const { return m_rayDistance > 0.f; }

    inline bool operator<(const RaycastResult& other) const
    {
        if (IsValid() && !other.IsValid()) return true;
        return m_rayDistance < other.m_rayDistance;
    }
};

/// <summary>
/// An ECS system that runs a simple 2D physics loop.
/// Use this for basic 2D collision detection/resolution with disks, polygons, and capsules.
/// For anything more advanced, consider using a professional physics engine instead, such as Jolt (see
/// bee::physics::JoltSystem).
/// </summary>
class World : public bee::System, bee::IEntityInspector
{
public:
    World(const float fixedDeltaTime) : m_fixedDeltaTime(fixedDeltaTime) {}

#ifdef BEE_INSPECTOR
    void OnEntity(bee::Entity entity) override;
#endif

    void Update(float dt) override;

    /// <summary>
    /// Sets the gravity vector that will be applied to all dynamic bodies in each frame.
    /// Note: this is not a force, but an acceleration. All dynamic bodies will experience it regardless of their mass.
    /// </summary>
    /// <param name="gravity">The desired acceleration vector, in meters per second squared.</param>
    void SetGravity(const glm::vec2& gravity) { m_gravity = gravity; }

    inline bool HasExecutedFrame() const { return m_hasExecutedFrame; }
    inline float GetFixedDeltaTime() const { return m_fixedDeltaTime; }

    /// <summary>
    /// Finds all physics objects that overlap with a given disk, and returns their entity IDs.
    /// </summary>
    /// <param name="pos">The center of the query disk.</param>
    /// <param name="radius">The radius of the query disk.</param>
    /// <returns>The IDs of all ECS entities that have a body+collider that overlaps with the given disk.</returns>
    static std::vector<bee::Entity> GetAllObjectsInRange(const glm::vec2& pos, float radius);

    /// <summary>
    /// Casts a 2D ray, and computes and returns information about the first physics object boundary that it hits.
    /// </summary>
    /// <param name="origin">The origin position of the ray.</param>
    /// <param name="direction">The direction vector of the ray.</param>
    /// <param name="result">Will store information about the first object that gets hit, if any.</param>
    /// <returns>The result of the raycast, or nothing if the ray does not hit any objects.</returns>
    static std::optional<RaycastResult> RaycastGetNearest(const glm::vec2& origin, const glm::vec2& direction);

    /// <summary>
    /// Casts a 2D ray, and computes and returns information about *all* physics objects of which the ray hits a boundary.
    /// </summary>
    /// <param name="origin">The origin position of the ray.</param>
    /// <param name="direction">The direction vector of the ray.</param>
    /// <param name="result">Will store information about the first object that gets hit, if any.</param>
    /// <returns>A list of RaycastResults (one for each object that was hit), sorted by distance from the ray's
    /// origin.</returns>
    static std::vector<RaycastResult> RaycastGetAll(const glm::vec2& origin, const glm::vec2& direction);

private:
    /// The fixed timestep (in seconds) for physics-related code.
    float m_fixedDeltaTime;
    float m_timeSinceLastFrame{0.f};
    bool m_hasExecutedFrame{false};
    glm::vec2 m_gravity{0, 0};

    static void ResolveCollision(const CollisionData& collision,
                                 Body& body1,
                                 Body& body2,
                                 PolygonCollider* polygon1 = nullptr,
                                 PolygonCollider* polygon2 = nullptr);
    static void RegisterCollision(CollisionData& collision,
                                  const bee::Entity& entity1,
                                  Body& body1,
                                  const bee::Entity& entity2,
                                  Body& body2);
    static void UpdateCollisionDetection();
};
}  // namespace bee::physics