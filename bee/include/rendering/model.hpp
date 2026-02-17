#pragma once

#include <tinygltf/tiny_gltf.h>

#include <memory>
#include <string>
#include <vector>

#include "core/ecs.hpp"
#include "core/resource.hpp"
#include "rendering/render_components.hpp"

namespace bee
{

struct Material;
class Mesh;
struct Texture;
struct Sampler;
struct Light;
class Image;

/// <summary>
/// Represents a model that can be read from a GLTF file.
/// </summary>
class Model : public Resource
{
public:
    /// <summary>
    /// Creates a new model by loading it from a given GLTF file.
    /// </summary>
    Model(FileIO::Directory directory, const std::string& filename);
    virtual ~Model();
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;
    Model(Model&&) = delete;
    Model& operator=(Model&&) = delete;

    const tinygltf::Model& GetDocument() const { return m_model; }
    const std::vector<std::vector<std::shared_ptr<Mesh>>>& GetMeshes() const { return m_meshes; }
    const std::vector<std::shared_ptr<Material>>& GetMaterials() const { return m_materials; }
    const std::vector<std::shared_ptr<Sampler>>& GetSamplers() const { return m_samplers; }
    const std::vector<std::shared_ptr<Light>>& GetLights() const { return m_lights; }

    /// <summary>
    /// Instantiates new entities and components with the contents of this model.
    /// </summary>
    /// <param name="parent">An optional parent entity. If specified, the newly created entities will be linked to this
    /// parent.</param>
    void Instantiate(Entity parent = entt::null) const;

    /// <summary>
    /// Creates a MeshRenderer containing one mesh and material from the GLTF node with the given name.
    /// If no node with this name exists in the model, the MeshRenderer will store nullptr for the mesh/material.
    /// NOTE: Use this function only if your GLTF model contains multiple meshes that you want to pick from.
    /// If you just want to instantiate the whole model (with its complete hierarchy of meshes), use Model::Instantiate()
    /// instead.
    /// </summary>
    /// <param name="nodeName">The name of the GLTF node to use.</param>
    /// <param name="primitiveIndex">The index of the primitive within the node to use. In a GLTF node, one mesh can consist of
    /// multiple primitives, each with their own material. In Bee, you would need multiple entities and MeshRenderers to render
    /// them all.</param>
    MeshRenderer CreateMeshRendererFromNode(const std::string& nodeName, size_t primitiveIndex = 0) const;

private:
    void InstantiateNode(uint32_t nodeIdx, Entity parent) const;

protected:
    tinygltf::Model m_model;
    std::vector<std::vector<std::shared_ptr<Mesh>>> m_meshes;
    std::vector<std::shared_ptr<Material>> m_materials;
    std::vector<std::shared_ptr<Sampler>> m_samplers;
    std::vector<std::shared_ptr<Light>> m_lights;
};

}  // namespace bee
