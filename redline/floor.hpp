#pragma once

#include <string>

#include "core/ecs.hpp"

class Floor
{
    bee::Entity ID;

public:
    Floor(float size, const std::string& texturePath, float tiling);

    bee::Entity GetEntity() const { return ID; }
};
