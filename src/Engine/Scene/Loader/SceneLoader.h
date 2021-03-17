#pragma once

#include <Types.h>

#include <Scene/SceneForwards.h>
#include <Scene/Components/ComponentsForwards.h>
#include <Scene/Loader/SceneDto.h>

#include <d3d12.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <vector>
#include <tuple>
#include <unordered_map>


struct aiScene;
struct aiNode;
struct aiMesh;
struct aiMaterial;
struct aiTexture;
struct aiString;
struct aiLight;
struct aiCamera;
enum aiTextureType;

namespace Engine::Scene::Loader
{
    class SceneLoader
    {
    private:
        struct LoadingContext
        {
            String RootPath;

            std::unordered_map<String, Index> imagesIndexMap;

            std::unordered_map<String, Index> lightsIndexMap;

            std::unordered_map<String, Index> camerasIndexMap;

            SceneDto* sceneDTO;
        };

    //! Values for the Sampler::magFilter field
    enum class SamplerMagFilter : unsigned int
    {
        UNSET = 0,
        SamplerMagFilter_Nearest = 9728,
        SamplerMagFilter_Linear = 9729
    };

    //! Values for the Sampler::minFilter field
    enum class SamplerMinFilter : unsigned int
    {
        UNSET = 0,
        SamplerMinFilter_Nearest = 9728,
        SamplerMinFilter_Linear = 9729,
        SamplerMinFilter_Nearest_Mipmap_Nearest = 9984,
        SamplerMinFilter_Linear_Mipmap_Nearest = 9985,
        SamplerMinFilter_Nearest_Mipmap_Linear = 9986,
        SamplerMinFilter_Linear_Mipmap_Linear = 9987
    };

    public:
        SceneDto LoadScene(String path, Optional<float32> scale = {});

    private:
        Node ParseNode(const aiNode* aNode, LoadingContext& context);

        bool IsLightNode(const aiNode* aNode, const LoadingContext& context);
        bool IsMeshNode(const aiNode* aNode, const LoadingContext& context);
        bool IsCameraNode(const aiNode* aNode, const LoadingContext& context);

        std::optional<Index> GetImage(const aiString& path, LoadingContext& context);

        SharedPtr<Image> ParseImage(const aiTexture* aTexture, const LoadingContext& context);
        CameraDto ParseCamera(const aiCamera* aCamera);
        LightDto ParseLight(const aiLight* aLight);
        MeshDto ParseMesh(const aiMesh* aMesh);
        MaterialDto ParseMaterial(const aiMaterial* aMaterial, LoadingContext& context);
        void ParseSampler(const aiMaterial* aMaterial, aiTextureType textureType, unsigned int idx);
    }; 
} // namespace Engine::Scene