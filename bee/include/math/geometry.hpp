#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

namespace bee
{
std::pair<glm::vec3, glm::vec3> ComputeAABB(const std::vector<glm::vec3>& pts);
}  // namespace bee