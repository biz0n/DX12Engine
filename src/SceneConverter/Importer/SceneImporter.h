#pragma once

#include <Model/Scene.h>

#include <d3d12.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>

#include <vector>
#include <tuple>
#include <unordered_map>
#include <map>


struct aiScene;
struct aiNode;
struct aiMesh;
struct aiMaterial;
struct aiTexture;
struct aiString;
struct aiLight;
struct aiCamera;
enum aiTextureType;

namespace SceneConverter::Importer
{
    class SceneImporter
    {
    private:
        struct LoadingContext
        {
            std::string RootPath;

            Model::Scene Scene;

            std::unordered_map<std::string, size_t> ImagesIndexMap;

            std::unordered_map<std::string, size_t> LightsIndexMap;

            std::unordered_map<std::string, size_t> CamerasIndexMap;
        };

    public:
        Model::Scene LoadScene(const std::string& path, std::optional<float> scale = {});

    private:
        Model::Node ParseNode(const aiNode* aNode, const aiScene* aScene, LoadingContext& context);

        bool IsLightNode(const aiNode* aNode, const LoadingContext& context);
        bool IsMeshNode(const aiNode* aNode, const LoadingContext& context);
        bool IsCameraNode(const aiNode* aNode, const LoadingContext& context);

        size_t GetImage(const aiString& path, LoadingContext& context, Model::TextureUsage usage);

        std::shared_ptr<Model::ImageData> ParseImage(const aiTexture* aTexture, const LoadingContext& context);
        Engine::Scene::Camera ParseCamera(const aiCamera* aCamera);
        Engine::Scene::PunctualLight ParseLight(const aiLight* aLight);
        Engine::Scene::Mesh ParseMesh(const aiMesh* aMesh, LoadingContext& context);
        Engine::Scene::Material ParseMaterial(const aiMaterial* aMaterial, LoadingContext& context);
        size_t ParseSampler(const aiMaterial* aMaterial, aiTextureType textureType, unsigned int idx, LoadingContext& context);
        DirectX::XMMATRIX LightMatrixFix(const aiLight* aLight, DirectX::XMMATRIX srt);
    }; 
} // namespace Engine::Scene