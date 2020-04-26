#pragma once

#include "Types.h"
#include "Resources/Shaders/ShaderTypes.h"

#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "Texture.h"
#include "Material.h"


#include <d3d12.h>
#include <DirectXMath.h>
#include <stdlib.h>
#include <vector>
#include <filesystem>
#include <array>
#include <unordered_map>

struct Vertex
{
    DirectX::XMFLOAT3 Vertex;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 TextureCoord;
    DirectX::XMFLOAT4 Tangent;

    static std::array<D3D12_INPUT_ELEMENT_DESC, 4> GetInputLayout()
    {
        return
        {
            {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            }
        };
    }
};

class Mesh
{
public:
    IndexBuffer mIndexBuffer;
    VertexBuffer mVertexBuffer;
    SharedPtr<Material> material;
};

class Node
{
public:
    Node *mParent;
    std::vector<SharedPtr<Mesh>> mMeshes;

    DirectX::XMFLOAT4X4 LocalTransform = DirectX::XMFLOAT4X4(1.f, 0.f, 0.f, 0.f,
                                                             0.f, 1.f, 0.f, 0.f,
                                                             0.f, 0.f, 1.f, 0.f,
                                                             0.f, 0.f, 0.f, 1.f);

    DirectX::XMMATRIX GetWorldTransform()
    {
        DirectX::XMMATRIX local = DirectX::XMLoadFloat4x4(&LocalTransform);
        if (mParent)
        {
            return DirectX::XMMatrixMultiply(mParent->GetWorldTransform(), local);
        }
        return local;
    }
};

struct aiScene;
struct aiNode;
struct aiMesh;
struct aiMaterial;
struct aiTexture;
struct aiString;

class Image;

class SceneObject
{
public:
    std::vector<UniquePtr<Node>> nodes;
};

class SceneLoader
{
private:
    struct LoadingContext
    {
        String RootPath;
        float32 Scale;
        std::vector<SharedPtr<Mesh>> meshes;
        std::vector<SharedPtr<Material>> materials;
        std::vector<SharedPtr<Texture>> textures;
        std::unordered_map<String, SharedPtr<Image>> images;
    };

public:
    UniquePtr<SceneObject> LoadScene(String path, float32 scale);

private:
    void ParseNode(const aiScene *aScene, aiNode *aNode, SceneObject *scene, Node *parentNode, const LoadingContext &context);
    SharedPtr<Texture> GetTexture(const aiString &path, LoadingContext &context);
    SharedPtr<Texture> GetTexture(const aiTexture *aTexture, const LoadingContext &context);
    SharedPtr<Material> ParseMaterial(const aiMaterial *aMaterial, LoadingContext &context);
    SharedPtr<Mesh> ParseMesh(const aiMesh *aMesh, const LoadingContext &context);
};