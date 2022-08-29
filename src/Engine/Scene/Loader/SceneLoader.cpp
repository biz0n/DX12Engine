#include "SceneLoader.h"

#include <StringUtils.h>

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>

#include <Scene/Image.h>
#include <Scene/Material.h>
#include <Scene/Vertex.h>
#include <Scene/CubeMap.h>
#include <Scene/SceneObject.h>
#include <Scene/Mesh.h>
#include <Scene/Camera.h>
#include <Scene/PunctualLight.h>
#include <Scene/Sampler.h>

#include <Scene/Components/RelationshipComponent.h>
#include <Scene/Components/LocalTransformComponent.h>
#include <Scene/Components/LightComponent.h>
#include <Scene/Components/CameraComponent.h>
#include <Scene/Components/MeshComponent.h>
#include <Scene/Components/NameComponent.h>
#include <Scene/Components/AABBComponent.h>

#include <Memory/IndexBuffer.h>
#include <Memory/VertexBuffer.h>

#include <filesystem>

#include <entt/entt.hpp>
#include <DirectXCollision.h>

namespace Engine::Scene::Loader
{
    SceneDto SceneLoader::LoadScene(String path, Optional<float32> scale)
    {
        std::filesystem::path filePath = path;
        std::filesystem::path exportPath = filePath;
        exportPath.replace_extension("assbin");

        Assimp::Importer importer;

        const aiScene *aScene;
       // if (std::filesystem::exists(exportPath) && std::filesystem::is_regular_file(exportPath))
        {
       //     aScene = importer.ReadFile(exportPath.string(), 0);
        }
        //else
        {
            unsigned int preprocessFlags = 0
                | aiProcess_Triangulate 
                | aiProcess_ConvertToLeftHanded
                //| aiProcess_JoinIdenticalVertices
                //| aiProcess_GenNormals
                | aiProcess_CalcTangentSpace
                | aiProcess_GenBoundingBoxes 
                //| aiProcess_OptimizeMeshes 
                //| aiProcess_OptimizeGraph
                ;
            if (scale.has_value())
            {
                preprocessFlags |= aiProcess_GlobalScale;
                importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, scale.value());
            }

            aScene = importer.ReadFile(filePath.string(), preprocessFlags);

           // if (aScene)
            {
                // Export the preprocessed scene file for faster loading next time.
                 //Assimp::Exporter exporter;
                 //exporter.Export(aScene, "assbin", exportPath.string(), preprocessFlags);
            }
        }

        SceneDto sceneDTO;

        LoadingContext context = {};
        context.RootPath = filePath.parent_path().string();
        context.sceneDTO = &sceneDTO;
        context.aScene = aScene;

        context.sceneDTO->ImageResources.reserve(aScene->mNumTextures);
        for (uint32 i = 0; i < aScene->mNumTextures; ++i)
        {
            aiTexture *aTexture = aScene->mTextures[i];
            auto texture = ParseImage(aTexture, context);
            context.sceneDTO->ImageResources.push_back({texture});
        }

        Sampler defaultSampler;
        defaultSampler.ModeU = Sampler::AddressMode::Mirror;
        defaultSampler.ModeV = Sampler::AddressMode::Mirror;
        defaultSampler.ModeW = Sampler::AddressMode::Mirror;
        defaultSampler.MinFilter = Sampler::Filter::Linear;
        defaultSampler.MagFilter = Sampler::Filter::Linear;
        defaultSampler.MipFilter = Sampler::Filter::Linear;

        context.defaultSamplerIndex = sceneDTO.Samplers.size();
        sceneDTO.Samplers.push_back(defaultSampler);
        size_t defaultSamplerHash = std::hash<Sampler>{}(defaultSampler);
        context.samplerHashesIndexMap[defaultSamplerHash] = context.defaultSamplerIndex;

        context.sceneDTO->Materials.reserve(static_cast<Size>(aScene->mNumMaterials));
        for (uint32 i = 0; i < aScene->mNumMaterials; ++i)
        {
            aiMaterial *aMaterial = aScene->mMaterials[i];
            auto material = ParseMaterial(aMaterial, context);
            context.sceneDTO->Materials.emplace_back(material);
        }

        context.sceneDTO->Meshes.reserve(static_cast<Size>(aScene->mNumMeshes));
        for (uint32 i = 0; i < aScene->mNumMeshes; ++i)
        {
            aiMesh *aMesh = aScene->mMeshes[i];
            auto mesh = ParseMesh(aMesh);
            context.sceneDTO->Meshes.push_back(mesh);
        }

        context.lightsIndexMap.reserve(static_cast<Size>(aScene->mNumLights));
        context.sceneDTO->Lights.reserve(static_cast<Size>(aScene->mNumLights));
        for (uint32 i = 0; i < aScene->mNumLights; ++i)
        {
            aiLight *aLight = aScene->mLights[i];
            context.lightsIndexMap[aLight->mName.C_Str()] = context.sceneDTO->Lights.size();
            context.sceneDTO->Lights.push_back(ParseLight(aLight));
        }

        context.camerasIndexMap.reserve(static_cast<Size>(aScene->mNumCameras));
        context.sceneDTO->Cameras.reserve(static_cast<Size>(aScene->mNumCameras));
		for (uint32 i = 0; i < aScene->mNumCameras; ++i)
		{
			aiCamera* aCamera = aScene->mCameras[i];
			context.camerasIndexMap[aCamera->mName.C_Str()] = context.sceneDTO->Cameras.size();
            context.sceneDTO->Cameras.push_back(ParseCamera(aCamera));
		}

        const auto rootNode = ParseNode(aScene->mRootNode, context);
		sceneDTO.RootNodes.push_back(rootNode);

        if (aScene->mNumCameras == 0)
        {
            CameraDto camera;
            camera.Name = "Default Camera";
            camera.Type = CameraType::Perspective;
            camera.NearPlane = 0.001f;
            camera.FarPlane = 100.0f;
            camera.FoV = 45 * Math::PI / 180.0f;

            Node cameraNode;
            cameraNode.Name = camera.Name;
            cameraNode.CameraIndex = sceneDTO.Cameras.size();
            cameraNode.LocalTransform = dx::XMMatrixIdentity();
            sceneDTO.Cameras.push_back(camera);
            sceneDTO.RootNodes.push_back(cameraNode);
        }

        std::function<void(const LightDto&, const dx::XMMATRIX&)> addLight = [&sceneDTO](const LightDto& light, const dx::XMMATRIX& transform)
        {
            Node lightNode;
            lightNode.Name = light.Name;
            lightNode.LocalTransform = transform;
            lightNode.LightIndex = sceneDTO.Lights.size();
            sceneDTO.Lights.push_back(light);
            sceneDTO.RootNodes.push_back(lightNode);
        };


        LightDto light1 = {};
        light1.Name = "Custom light 1";
        light1.Color = {1.0f, 0.6f, 0.2f};
        light1.Intensity = 50;
        light1.ConstantAttenuation = 0;
        light1.LinearAttenuation = 0;
        light1.QuadraticAttenuation = 1;
        light1.InnerConeAngle = 0;
        light1.OuterConeAngle = 0;
        light1.LightType = LightType::PointLight;

       // addLight(light1, DirectX::XMMatrixTranslation(4.0f, 5.0f, -2.0f));

        LightDto light2 = {};
        light2.Name = "Custom light 2";
        light2.Color = {1.0f, 1.0f, 1.0f};
        light2.Intensity = 2;
        light2.ConstantAttenuation = 0;
        light2.LinearAttenuation = 0;
        light2.QuadraticAttenuation = 1;
        light2.InnerConeAngle = 0;
        light2.OuterConeAngle = 0;
        light2.LightType = LightType::DirectionalLight;

       // addLight(light2, DirectX::XMMatrixTranslation(0.0f, 2.0f, 0.0f));

        return sceneDTO;
    }

    Node SceneLoader::ParseNode(const aiNode *aNode, LoadingContext &context)
    {
        aiVector3D scaling;
		aiQuaternion rotation;
		aiVector3D position;
		aNode->mTransformation.Decompose(scaling, rotation, position);

		auto t = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
		auto s = DirectX::XMMatrixScaling(scaling.x, scaling.y, scaling.z);
		auto r = DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w));

        DirectX::XMMATRIX srt = s * r * t;

        String name = aNode->mName.C_Str();
        Node node;


		if (IsMeshNode(aNode, context))
		{
            node.Name = "Mesh_" + name;
            node.LocalTransform = srt;
            for (uint32 i = 0; i < aNode->mNumMeshes; ++i)
            {
                uint32 meshIndex = aNode->mMeshes[i];
                node.MeshIndices.push_back(meshIndex);
            }
		}
        else if (IsLightNode(aNode, context))
        {
            node.Name = "Light_" + name;
            node.LightIndex = context.lightsIndexMap[name];
            const auto* light = context.aScene->mLights[*node.LightIndex];
            node.LocalTransform = LightMatrixFix(light, srt);
        }
        else if (IsCameraNode(aNode, context))
        {
            node.Name = "Camera_" + name;
            node.LocalTransform = srt;
            node.CameraIndex = context.camerasIndexMap[name];
        }
        else
        {
            node.Name = name;
            node.LocalTransform = srt;

            for (auto i = 0; i < aNode->mNumChildren; i++)
            {
                auto aChild = aNode->mChildren[i];
                Node child = ParseNode(aChild, context);
                node.Children.push_back(child);
            }
        }

        return node;
    }

    Index SceneLoader::ParseSampler(const aiMaterial* aMaterial, aiTextureType textureType, unsigned int idx, LoadingContext& context)
    {
        Sampler sampler = context.sceneDTO->Samplers[context.defaultSamplerIndex];

        aiTextureMapMode wrapS;
        if (aMaterial->Get<aiTextureMapMode>(AI_MATKEY_MAPPINGMODE_U(textureType, idx), wrapS) == aiReturn_SUCCESS)
        {
            switch (wrapS)
            {
                case aiTextureMapMode_Wrap:
                    sampler.ModeU = Sampler::AddressMode::Wrap;
                    break;
                case aiTextureMapMode_Clamp:
                    sampler.ModeU = Sampler::AddressMode::Clamp;
                    break;
                case aiTextureMapMode_Decal:
                    sampler.ModeU = Sampler::AddressMode::Border;
                    break;
                case aiTextureMapMode_Mirror:
                    sampler.ModeU = Sampler::AddressMode::Mirror;
                    break;
                default:
                    sampler.ModeU = Sampler::AddressMode::Wrap;
                    break;
            }
        }

        aiTextureMapMode wrapT;
        if (aMaterial->Get<aiTextureMapMode>(AI_MATKEY_MAPPINGMODE_V(textureType, idx), wrapT) == aiReturn_SUCCESS)
        {
            switch (wrapT)
            {
                case aiTextureMapMode_Wrap:
                    sampler.ModeV = Sampler::AddressMode::Wrap;
                    break;
                case aiTextureMapMode_Clamp:
                    sampler.ModeV = Sampler::AddressMode::Clamp;
                    break;
                case aiTextureMapMode_Decal:
                    sampler.ModeV = Sampler::AddressMode::Border;
                    break;
                case aiTextureMapMode_Mirror:
                    sampler.ModeV = Sampler::AddressMode::Mirror;
                    break;
                default:
                    sampler.ModeV = Sampler::AddressMode::Wrap;
                    break;
            }
        }

        SamplerMagFilter magFilter;
        if (aMaterial->Get<SamplerMagFilter>(AI_MATKEY_GLTF_MAPPINGFILTER_MAG(textureType, idx), magFilter) == aiReturn_SUCCESS)
        {
            switch (magFilter)
            {
                case SamplerMagFilter::SamplerMagFilter_Nearest:
                    sampler.MagFilter = Sampler::Filter::Point;
                    break;
                case SamplerMagFilter::SamplerMagFilter_Linear:
                    sampler.MagFilter = Sampler::Filter::Linear;
                    break;
            }
        }

        SamplerMinFilter minFilter;
        if (aMaterial->Get<SamplerMinFilter>(AI_MATKEY_GLTF_MAPPINGFILTER_MIN(textureType, idx), minFilter) == aiReturn_SUCCESS)
        {
            switch (minFilter)
            {
                case SamplerMinFilter::SamplerMinFilter_Nearest:
                    sampler.MinFilter = Sampler::Filter::Point;
                    sampler.MipFilter = Sampler::Filter::Point;
                    sampler.MinLod = 0;
                    sampler.MaxLod = 0.25;
                    break;
                case SamplerMinFilter::SamplerMinFilter_Linear:
                    sampler.MinFilter = Sampler::Filter::Linear;
                    sampler.MipFilter = Sampler::Filter::Linear;
                    sampler.MinLod = 0;
                    sampler.MaxLod = 0.25;
                    break;
                case SamplerMinFilter::SamplerMinFilter_Nearest_Mipmap_Nearest:
                    sampler.MinFilter = Sampler::Filter::Point;
                    sampler.MipFilter = Sampler::Filter::Point;
                    break;
                case SamplerMinFilter::SamplerMinFilter_Linear_Mipmap_Nearest:
                    sampler.MinFilter = Sampler::Filter::Linear;
                    sampler.MipFilter = Sampler::Filter::Point;
                    break;
                case SamplerMinFilter::SamplerMinFilter_Nearest_Mipmap_Linear:
                    sampler.MinFilter = Sampler::Filter::Point;
                    sampler.MipFilter = Sampler::Filter::Linear;
                    break;
                case SamplerMinFilter::SamplerMinFilter_Linear_Mipmap_Linear:
                    sampler.MinFilter = Sampler::Filter::Linear;
                    sampler.MipFilter = Sampler::Filter::Linear;
                    break;
            }
        }

        size_t hash = std::hash<Sampler>{}(sampler);
        auto iter = context.samplerHashesIndexMap.find(hash);
        if (iter != context.samplerHashesIndexMap.end())
        {
            return iter->second;
        }
        else
        {
            Index index = context.sceneDTO->Samplers.size();
            context.samplerHashesIndexMap[hash] = index;
            context.sceneDTO->Samplers.push_back(sampler);
            return index;
        }
    }

    SharedPtr<Image> SceneLoader::ParseImage(const aiTexture *aTexture, const LoadingContext &context)
    {
        std::vector<Byte> buffer;
        buffer.resize(aTexture->mWidth);
        memcpy(buffer.data(), aTexture->pcData, aTexture->mWidth);
        String name = context.RootPath + "\\" + aTexture->mFilename.C_Str();
        SharedPtr<Scene::Image> image = Scene::Image::LoadImageFromData(buffer, aTexture->achFormatHint, name);

        return image;
    }

    std::optional<Index> SceneLoader::GetImage(const aiString &path, LoadingContext &context)
    {
        if (path.C_Str()[0] != '*')
        {
            std::filesystem::path filePath = context.RootPath + "\\" + path.C_Str();

            String filePathStr = filePath.string();
            auto iter = context.imagesIndexMap.find(filePathStr);
            if (iter != context.imagesIndexMap.end())
            {
                return iter->second;
            }
            else
            {
                if (std::filesystem::exists(filePath))
                {
                    auto image = Scene::Image::LoadImageFromFile(filePathStr);
                    auto index = context.sceneDTO->ImageResources.size();
                    context.imagesIndexMap[filePathStr] = index;
                    context.sceneDTO->ImageResources.push_back({image});

                    return index;
                }
            }
        }
        else
        {
            int index = atoi(path.C_Str() + 1);
            return { index };
        }

        return std::nullopt;
    }

	bool SceneLoader::IsLightNode(const aiNode* aNode, const LoadingContext& context)
	{
		auto iter = context.lightsIndexMap.find(aNode->mName.C_Str());
        return iter != context.lightsIndexMap.end();
	}

	bool SceneLoader::IsMeshNode(const aiNode* aNode, const LoadingContext& context)
	{
        return aNode->mNumMeshes > 0;
	}

    bool SceneLoader::IsCameraNode(const aiNode* aNode, const LoadingContext& context)
	{
		auto iter = context.camerasIndexMap.find(aNode->mName.C_Str());
        return iter != context.camerasIndexMap.end();
	}

    CameraDto SceneLoader::ParseCamera(const aiCamera* aCamera)
    {
        CameraDto camera;

        camera.Name = aCamera->mName.C_Str();
        camera.FoV = aCamera->mHorizontalFOV;
        camera.FarPlane = aCamera->mClipPlaneFar;
        camera.NearPlane = aCamera->mClipPlaneNear;
        camera.Type = CameraType::Perspective;

        return camera;
    }

    LightDto SceneLoader::ParseLight(const aiLight* aLight)
    {
        LightDto light;
        light.Name = aLight->mName.C_Str();

        switch (aLight->mType)
        {
            case aiLightSource_DIRECTIONAL:
                light.LightType = LightType::DirectionalLight;
                break;
            case aiLightSource_POINT:
                light.LightType = LightType::PointLight;
                break;
            case aiLightSource_SPOT:
                light.LightType = LightType::SpotLight;
                break;
            default:
                // add ambient light
                light.LightType = LightType::SpotLight;
                break;
        }

        float intensity = std::max(1.0f, std::max(aLight->mColorDiffuse.r, std::max(aLight->mColorDiffuse.g, aLight->mColorDiffuse.b)));
        DirectX::XMFLOAT3 color{aLight->mColorDiffuse.r / intensity, aLight->mColorDiffuse.g / intensity, aLight->mColorDiffuse.b / intensity};
        light.Color = color;
        light.Intensity = intensity;

        light.ConstantAttenuation = aLight->mAttenuationConstant;
        light.LinearAttenuation = aLight->mAttenuationLinear;
        light.QuadraticAttenuation = aLight->mAttenuationQuadratic;

        light.InnerConeAngle = aLight->mAngleInnerCone;
        light.OuterConeAngle = aLight->mAngleOuterCone;

        return light;
    }

    MeshDto SceneLoader::ParseMesh(const aiMesh* aMesh)
    {
        MeshDto mesh;
        mesh.Name = aMesh->mName.C_Str();

        std::vector<Vertex> vertices;
        vertices.reserve(aMesh->mNumVertices);

        for (uint32 i = 0; i < aMesh->mNumVertices; ++i)
        {
            Vertex vertex = {};
            vertex.Vertex = *reinterpret_cast<DirectX::XMFLOAT3 *>(&aMesh->mVertices[i]);
            vertex.Normal = *reinterpret_cast<DirectX::XMFLOAT3 *>(&aMesh->mNormals[i]);
            if (aMesh->mTextureCoords[0] != nullptr)
            {
                vertex.TextureCoord = *reinterpret_cast<DirectX::XMFLOAT2 *>(&aMesh->mTextureCoords[0][i]);
            }
            else
            {
                vertex.TextureCoord = {0.0f, 0.0f};
            }
            if (aMesh->mTangents != nullptr && aMesh->mBitangents != nullptr)
            {
                auto tangent = *reinterpret_cast<DirectX::XMFLOAT3 *>(&aMesh->mTangents[i]);
                auto biTangent = *reinterpret_cast<DirectX::XMFLOAT3 *>(&aMesh->mBitangents[i]);
                float32 symmetry = +1.0f;
                if (DirectX::XMVectorGetX(DirectX::XMVector3Dot(
                        DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&vertex.Normal), DirectX::XMLoadFloat3(&tangent)),
                        DirectX::XMLoadFloat3(&biTangent))) < 0.0f)
                {
                    symmetry = -1.0f;
                }

                vertex.Tangent = {tangent.x, tangent.y, tangent.z, symmetry};
            }
            vertices.emplace_back(vertex);
        }

        mesh.Vertices = std::move(vertices) ;

        std::vector<uint32> indices;
        indices.reserve(aMesh->mNumFaces * 3);
        for (uint32 i = 0; i < aMesh->mNumFaces; ++i)
        {
            const auto &face = aMesh->mFaces[i];
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
        }

        mesh.Indices = std::move(indices);

        mesh.MaterialIndex = aMesh->mMaterialIndex;

        const auto& aabbMin = aMesh->mAABB.mMin;
        const auto& aabbMax = aMesh->mAABB.mMax;

        dx::BoundingBox boundingBox;
        dx::BoundingBox::CreateFromPoints(
                boundingBox,
                dx::XMVectorSet(aabbMin.x, aabbMin.y, aabbMin.z, 0.0),
                dx::XMVectorSet(aabbMax.x, aabbMax.y, aabbMax.z, 0.0));

        mesh.AABB = boundingBox;

        return mesh;
    }

    MaterialDto SceneLoader::ParseMaterial(const aiMaterial* aMaterial, LoadingContext& context)
    {
        MaterialDto material;
        MaterialProperties properties;

        aiString name;
        aMaterial->Get(AI_MATKEY_NAME, name);

        aiColor4D baseColor;
        if (aMaterial->Get(AI_MATKEY_BASE_COLOR, baseColor) == aiReturn_SUCCESS)
        {
            properties.baseColor.baseColor = *reinterpret_cast<DirectX::XMFLOAT4 *>(&baseColor);
        }

        float32 metallicFactor = 0;
        if (aMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor) == aiReturn_SUCCESS)
        {
            properties.metallicRaughness.metallicFactor = metallicFactor;
        }

        float32 roughnessFactor = 0;
        if (aMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor) == aiReturn_SUCCESS)
        {
            properties.metallicRaughness.roughnessFactor = roughnessFactor;
        }

        aiColor3D emissiveFactor;
        if (aMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveFactor) == aiReturn_SUCCESS)
        {
            properties.emissive.factor = *reinterpret_cast<DirectX::XMFLOAT3 *>(&emissiveFactor);
        }

        float32 alphaCutoff = 0.5f;
        if (aMaterial->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCutoff) == aiReturn_SUCCESS)
        {
            properties.alphaCutoff = alphaCutoff;
        }

        aiString alphaMode;
        if (aMaterial->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode) == aiReturn_SUCCESS)
        {
            auto alphaModeStr = String(alphaMode.C_Str());
            if (alphaModeStr == "OPAQUE")
            {
                properties.alphaMode = AlphaMode::Opaque;
            }
            else if (alphaModeStr == "MASK")
            {
                properties.alphaMode = AlphaMode::Mask;
            }
            else if (alphaModeStr == "BLEND")
            {
                properties.alphaMode = AlphaMode::Blend;
            }
        }

        aiShadingMode shadingMode = aiShadingMode_PBR_BRDF;
        if (aMaterial->Get(AI_MATKEY_SHADING_MODEL, shadingMode) == aiReturn_SUCCESS)
        {
            properties.unlit = shadingMode == aiShadingMode_Unlit;
        }

        bool twoSided;
        if (aMaterial->Get(AI_MATKEY_TWOSIDED, twoSided) == aiReturn_SUCCESS)
        {
            properties.doubleSided = twoSided;
        }

        aiString albedoTexturePath;
        if (aMaterial->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &albedoTexturePath) == aiReturn_SUCCESS)
        {
            material.BaseColorTextureSamplerIndex = ParseSampler(aMaterial, AI_MATKEY_BASE_COLOR_TEXTURE, context);
            auto albedoTextureIndex = GetImage(albedoTexturePath, context);
            material.BaseColorTextureIndex = albedoTextureIndex;
        }

        aiString normalTexturePath;
        if (aMaterial->GetTexture(aiTextureType_NORMALS, 0, &normalTexturePath) == aiReturn_SUCCESS)
        {
            material.NormalTextureSamplerIndex = ParseSampler(aMaterial, aiTextureType_NORMALS, 0, context);
            auto normalTextureIndex = GetImage(normalTexturePath, context);
            material.NormalTextureIndex = normalTextureIndex;

            float32 scale;
            if (aMaterial->Get(AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_NORMALS, 0), scale) == aiReturn_SUCCESS)
            {
                properties.normalTextureInfo.scale = scale;
            }
        }

        aiString metallicRoughnessTexturePath;
        if (aMaterial->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &metallicRoughnessTexturePath) == aiReturn_SUCCESS)
        {
            material.MetallicRoughnessTextureSamplerIndex = ParseSampler(aMaterial, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, context);
            auto metallicRoughnessTextureIndex = GetImage(metallicRoughnessTexturePath, context);
            material.MetallicRoughnessTextureIndex = metallicRoughnessTextureIndex;
        }

        aiString ambientOcclusionTexturePath;
        if (aMaterial->GetTexture(aiTextureType_LIGHTMAP, 0, &ambientOcclusionTexturePath) == aiReturn_SUCCESS)
        {
            material.AmbientOcclusionTextureSamplerIndex = ParseSampler(aMaterial, aiTextureType_LIGHTMAP, 0, context);
            auto aoTextureIndex = GetImage(ambientOcclusionTexturePath, context);
            material.AmbientOcclusionTextureIndex = aoTextureIndex;
        }

        aiString emissiveTexturePath;
        if (aMaterial->GetTexture(aiTextureType_EMISSIVE, 0, &emissiveTexturePath) == aiReturn_SUCCESS)
        {
            material.EmissiveTextureSamplerIndex = ParseSampler(aMaterial, aiTextureType_EMISSIVE, 0, context);
            auto emissiveTextureIndex = GetImage(emissiveTexturePath, context);
            material.EmissiveTextureIndex = emissiveTextureIndex;

            float32 strength;
            if (aMaterial->Get(AI_MATKEY_GLTF_TEXTURE_STRENGTH(aiTextureType_EMISSIVE, 0), strength) == aiReturn_SUCCESS)
            {
                properties.emissive.info.strength = strength;
            }
        }

        material.MaterialProperties = properties;

        return material;
    }

    DirectX::XMMATRIX SceneLoader::LightMatrixFix(const aiLight* aLight, DirectX::XMMATRIX srt)
    {
        dx::XMVECTOR direction = dx::XMVectorSet(aLight->mDirection.x, aLight->mDirection.z, aLight->mDirection.y, 0.f );
        dx::XMVECTOR up = dx::XMVectorSet( aLight->mUp.x, aLight->mUp.z, aLight->mUp.y, 0.f );

        if (aLight->mType != aiLightSource_POINT)
        {
            auto local = dx::XMMatrixLookToLH(dx::XMVectorZero(), direction, up);

            dx::XMVECTOR d;
            auto inv = dx::XMMatrixInverse(&d, local);

            return dx::XMMatrixMultiply(inv, srt);
        }
        else
        {
            return srt;
        }
    }

} // namespace Engine::Scene::Loader