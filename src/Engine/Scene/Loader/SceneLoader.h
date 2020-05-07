#pragma once

#include <Types.h>
#include <ShaderTypes.h>

#include <IndexBuffer.h>
#include <VertexBuffer.h>
#include <Scene/Texture.h>
#include <Scene/Material.h>

#include <d3d12.h>
#include <DirectXMath.h>
#include <stdlib.h>
#include <vector>
#include <filesystem>
#include <array>
#include <unordered_map>

struct aiScene;
struct aiNode;
struct aiMesh;
struct aiMaterial;
struct aiTexture;
struct aiString;
struct aiLight;

namespace Engine::Scene
{
    struct Vertex
    {
        DirectX::XMFLOAT3 Vertex;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT2 TextureCoord;
        DirectX::XMFLOAT4 Tangent;

        static std::array<D3D12_INPUT_ELEMENT_DESC, 4> GetInputLayout()
        {
            return {
                {
                    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                }};
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

        DirectX::XMMATRIX GetWorldTransform() const
        {
            DirectX::XMMATRIX local = DirectX::XMLoadFloat4x4(&LocalTransform);
            if (mParent)
            {
                return DirectX::XMMatrixMultiply(mParent->GetWorldTransform(), local);
            }
            return local;
        }
    };

    class LightNode : public Node
    {
    public:
        void SetLightType(int lightType) { mLightType = lightType; }
        int GetLightType() const { return mLightType; }

        void SetEnabled(bool enabled) { mEnabled = enabled; }
        bool IsEnabled() const { return mEnabled; }

        void SetDirection(const DirectX::XMFLOAT3 &direction) { mDirection = direction; }
        const DirectX::XMFLOAT3 &GetDirection() const { return mDirection; }

        void SetColor(const DirectX::XMFLOAT3 &color) { mColor = color; }
        const DirectX::XMFLOAT3 &GetColor() const { return mColor; }

        void SetConstantAttenuation(float attenuation) { mConstantAttenuation = attenuation; }
        float GetConstantAttenuation() const { return mConstantAttenuation; }

        void SetLinearAttenuation(float attenuation) { mLinearAttenuation = attenuation; }
        float GetLinearAttenuation() const { return mLinearAttenuation; }

        void SetQuadraticAttenuation(float attenuation) { mQuadraticAttenuation = attenuation; }
        float GetQuadraticAttenuation() const { return mQuadraticAttenuation; }

        void SetInnerConeAngle(float coneAngle) { mInnerConeAngle = coneAngle; }
        float GetInnerConeAngle() const { return mInnerConeAngle; }

        void SetOuterConeAngle(float coneAngle) { mOuterConeAngle = coneAngle; }
        float GetOuterConeAngle() const { return mOuterConeAngle; }

    private:
        int mLightType;
        bool mEnabled;
        DirectX::XMFLOAT3 mDirection;
        DirectX::XMFLOAT3 mColor;
        float mConstantAttenuation;
        float mLinearAttenuation;
        float mQuadraticAttenuation;
        float mInnerConeAngle;
        float mOuterConeAngle;
    };

    class Image;

    class SceneObject
    {
    public:
        std::vector<UniquePtr<Node>> nodes;

        std::vector<UniquePtr<LightNode>> lights;
    };
    namespace Loader
    {
        class SceneLoader
        {
        private:
            struct LoadingContext
            {
                String RootPath;
                std::vector<SharedPtr<Mesh>> meshes;
                std::vector<SharedPtr<Material>> materials;
                std::vector<SharedPtr<Texture>> textures;
                std::unordered_map<String, SharedPtr<Scene::Image>> images;
                std::unordered_map<String, aiLight *> lightsMap;
            };

        public:
            UniquePtr<SceneObject> LoadScene(String path, float32 scale);

        private:
            void ParseNode(const aiScene *aScene, aiNode *aNode, SceneObject *scene, Node *parentNode, const LoadingContext &context);
            SharedPtr<Texture> GetTexture(const aiString &path, LoadingContext &context);
            SharedPtr<Texture> GetTexture(const aiTexture *aTexture, const LoadingContext &context);
            SharedPtr<Material> ParseMaterial(const aiMaterial *aMaterial, LoadingContext &context);
            SharedPtr<Mesh> ParseMesh(const aiMesh *aMesh, const LoadingContext &context);
            UniquePtr<LightNode> ParseLight(const aiLight *aLight, const LoadingContext &context);
        }; // namespace LoaderclassSceneLoader

    } // namespace Loader
} // namespace Engine::Scene