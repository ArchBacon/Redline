#include "rendering/render_components.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>

#include "math/geometry.hpp"
#include "tools/tools.hpp"
#include "rendering/model.hpp"
#include "rendering/image.hpp"
#include "core/engine.hpp"
#include "core/resources.hpp"

using namespace bee;
using namespace std;

Material::Material(const Model& model, int index)
{
    const auto& material = model.GetDocument().materials[index];

    EmissiveFactor = to_vec3(material.emissiveFactor);

    if (material.emissiveTexture.index != -1)
    {
        EmissiveTexture = make_shared<Texture>(model, material.emissiveTexture.index, Image::Format::kSrgb);
        UseEmissiveTexture = true;
    }

    if (material.normalTexture.index != -1)
    {
        NormalTexture = make_shared<Texture>(model, material.normalTexture.index, Image::Format::kUnorm);
        NormalTextureScale = (float)material.normalTexture.scale;
        UseNormalTexture = true;
    }

    if (material.occlusionTexture.index != -1)
    {
        OcclusionTexture = make_shared<Texture>(model, material.occlusionTexture.index, Image::Format::kUnorm);
        OcclusionTextureStrength = (float)material.occlusionTexture.strength;
        UseOcclusionTexture = true;
    }

    {
        const auto& pbr = material.pbrMetallicRoughness;
        BaseColorFactor = to_vec4(pbr.baseColorFactor);
        if (pbr.baseColorTexture.index != -1)
        {
            BaseColorTexture = make_shared<Texture>(model, pbr.baseColorTexture.index, Image::Format::kSrgb);
            UseBaseTexture = true;
        }

        if (pbr.metallicRoughnessTexture.index != -1)
        {
            MetallicRoughnessTexture = make_shared<Texture>(model, pbr.metallicRoughnessTexture.index, Image::Format::kUnorm);
            UseMetallicRoughnessTexture = true;
        }

        MetallicFactor = (float)pbr.metallicFactor;
        RoughnessFactor = (float)pbr.roughnessFactor;
    }

    if (material.extensions.find("KHR_materials_unlit") != material.extensions.end())
    {
        IsUnlit = true;
    }
}

Sampler::Sampler(const Model& model, int index)
{
    const auto& sampler = model.GetDocument().samplers[index];
    MagFilter = GetFilter(sampler.magFilter);
    MinFilter = GetFilter(sampler.minFilter);
    WrapS = GetWrap(sampler.wrapS);
    WrapT = GetWrap(sampler.wrapT);
}

Sampler::Filter Sampler::GetFilter(int filter)
{
    switch (filter)
    {
        case TINYGLTF_TEXTURE_FILTER_NEAREST:
            return Filter::Nearest;
        case TINYGLTF_TEXTURE_FILTER_LINEAR:
            return Filter::Linear;
        case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
            return Filter::NearestMipmapNearest;
        case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
            return Filter::LinearMipmapNearest;
        case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
            return Filter::NearestMipmapLinear;
        case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
            return Filter::LinearMipmapLinear;
        default:
            return Filter::Nearest;
    }
}

Sampler::Wrap Sampler::GetWrap(int wrap)
{
    switch (wrap)
    {
        case TINYGLTF_TEXTURE_WRAP_REPEAT:
            return Wrap::Repeat;
        case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
            return Wrap::ClampToEdge;
        case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
            return Wrap::MirroredRepeat;
        default:
            return Wrap::ClampToEdge;
    }
}

Sampler::Sampler() = default;

Texture::Texture(const Model& model, int index, bee::Image::Format format)
{
    const auto& texture = model.GetDocument().textures[index];
    assert(texture.source != -1);
    Image = Engine.Resources().Load<bee::Image>(model, index, format);

    if (texture.sampler != -1)
        Sampler = model.GetSamplers()[texture.sampler];
    else
        Sampler = make_shared<bee::Sampler>();
}

Light::Light(const Model& model, int index)
{
    const auto& light = model.GetDocument().lights[index];
    Color = to_vec3(light.color);
    Intensity = (float)light.intensity;
    Range = (float)light.range;
    if (light.type == "point")
        Type = Type::Point;
    else if (light.type == "directional")
        Type = Type::Directional;
    else
        Type = Type::Spot;
}