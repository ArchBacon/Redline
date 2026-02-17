#include "math/geometry.hpp"

using namespace glm;
using namespace bee;

std::pair<glm::vec3, glm::vec3> bee::ComputeAABB(const std::vector<glm::vec3>& pts)
{
    glm::vec3 min = pts[0], max = pts[0];
    for (const auto& pt : pts)
    {
        if (pt.x < min.x) min.x = pt.x;
        if (pt.y < min.y) min.y = pt.y;
        if (pt.z < min.z) min.z = pt.z;
        if (pt.x > max.x) max.x = pt.x;
        if (pt.y > max.y) max.y = pt.y;
        if (pt.z > max.z) max.z = pt.z;
    }
    return {min, max};
}