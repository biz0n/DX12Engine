#pragma once

#include <Types.h>

#include <Scene/Material.h>
#include <Scene/PunctualLight.h>
#include <Scene/Camera.h>
#include <Scene/Sampler.h>
#include <Scene/Vertex.h>

#include <vector>
#include <optional>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <d3d12.h>

namespace Engine::Scene::Loader
{
    struct MeshDto
    {
        String Name;
        std::vector<Vertex> Vertices;
        std::vector<uint32> Indices;
        D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology;
        Index MaterialIndex;
        dx::BoundingBox AABB;
    };

    struct MaterialDto
    {
        MaterialProperties MaterialProperties;
        std::optional<Index> BaseColorTextureIndex;
        std::optional<Index> NormalTextureIndex;
        std::optional<Index> MetallicRoughnessTextureIndex;
        std::optional<Index> AmbientOcclusionTextureIndex;
        std::optional<Index> EmissiveTextureIndex;

        std::optional<Index> BaseColorTextureSamplerIndex;
        std::optional<Index> NormalTextureSamplerIndex;
        std::optional<Index> MetallicRoughnessTextureSamplerIndex;
        std::optional<Index> AmbientOcclusionTextureSamplerIndex;
        std::optional<Index> EmissiveTextureSamplerIndex;
    };

    struct LightDto
    {
        String Name;
        LightType LightType;
        DirectX::XMFLOAT3 Direction;
        DirectX::XMFLOAT3 Color;
        float Intensity;
        float ConstantAttenuation;
        float LinearAttenuation;
        float QuadraticAttenuation;
        float InnerConeAngle;
        float OuterConeAngle;
    };

    struct CameraDto
    {
        String Name;
        CameraType Type;
        float32 NearPlane;
        float32 FarPlane;
        float32 FoV;
    };

    struct ImageDto
    {
        SharedPtr<Image> Image;
    };

    struct Node;
    struct Node
    {
        std::vector<Node> Children;
        std::vector<Index> MeshIndices;
        std::optional<Index> LightIndex;
        std::optional<Index> CameraIndex;
        dx::XMMATRIX LocalTransform;
        String Name;
    };

    struct SceneDto
    {
        using SamplerDTO = Sampler;

        std::vector<MeshDto> Meshes;
        std::vector<MaterialDto> Materials;
        std::vector<LightDto> Lights;
        std::vector<CameraDto> Cameras;
        std::vector<SamplerDTO> Samplers;
        std::vector<ImageDto> ImageResources;

        std::vector<Node> RootNodes;

    };
}