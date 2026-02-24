#include "floor.hpp"

#include <glm/glm.hpp>

#include "core/engine.hpp"
#include "core/resources.hpp"
#include "core/transform.hpp"
#include "platform/opengl/mesh_gl.hpp"
#include "platform/opengl/image_gl.hpp"
#include "rendering/render_components.hpp"

Floor::Floor(float size, const std::string& texturePath, float tiling)
{
    ID = bee::Engine.ECS().CreateEntity();
    auto& transform = bee::Engine.ECS().CreateComponent<bee::Transform>(ID);
    transform.Name = "Floor";

    std::vector<glm::vec3> positions = {
        {-size, -size, 0.0f},
        { size, -size, 0.0f},
        { size,  size, 0.0f},
        {-size,  size, 0.0f},
    };
    std::vector<glm::vec3> normals = {
        {0,0,1}, {0,0,1}, {0,0,1}, {0,0,1}
    };
    std::vector<glm::vec2> uvs = {
        {0,      0      },
        {tiling, 0      },
        {tiling, tiling },
        {0,      tiling },
    };
    std::vector<uint16_t> indices = { 0, 1, 2, 0, 2, 3 };

    auto mesh = std::make_shared<bee::Mesh>();
    mesh->SetAttribute(bee::Mesh::Attribute::Position, positions);
    mesh->SetAttribute(bee::Mesh::Attribute::Normal,   normals);
    mesh->SetAttribute(bee::Mesh::Attribute::Texture,  uvs);
    mesh->SetIndices(indices);

    auto image = bee::Engine.Resources().Load<bee::Image>(
        bee::FileIO::Directory::Assets,
        texturePath,
        bee::Image::Format::kSrgb
    );

    auto sampler = std::make_shared<bee::Sampler>();
    sampler->MagFilter = bee::Sampler::Filter::Linear;
    sampler->MinFilter = bee::Sampler::Filter::LinearMipmapLinear;
    sampler->WrapS     = bee::Sampler::Wrap::Repeat;
    sampler->WrapT     = bee::Sampler::Wrap::Repeat;

    auto texture  = std::make_shared<bee::Texture>(image, sampler);
    auto material = std::make_shared<bee::Material>();
    material->BaseColorTexture = texture;
    material->UseBaseTexture   = true;
    material->RoughnessFactor  = 0.8f;

    auto& mr = bee::Engine.ECS().CreateComponent<bee::MeshRenderer>(ID);
    mr.Mesh     = mesh;
    mr.Material = material;
}
