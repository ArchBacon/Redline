#pragma once

#include <glm/vec3.hpp>
#include "core/ecs.hpp"

struct Vehicle {};

class BuickGrandNational87
{
    bee::Entity ID;

public:
    BuickGrandNational87();

    bee::Entity GetEntity() const { return ID; }
    
    bee::Entity CreateCarBody();
    void CreateCarWheel(bee::Entity parent, const std::string& affix, const glm::vec3& position, bool mirror);
};
