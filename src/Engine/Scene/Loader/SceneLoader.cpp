#include "SceneLoader.h"

#include <StringUtils.h>
#include <Scene/Image.h>
#include <Scene/Texture.h>
#include <Scene/Material.h>
#include <Scene/Vertex.h>

#include <Scene/SceneObject.h>
#include <Scene/Mesh.h>

#include <Scene/Camera.h>
#include <Scene/PunctualLight.h>

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>

#include <Scene/Components/RelationshipComponent.h>
#include <Scene/Components/LocalTransformComponent.h>
#include <Scene/Components/LightComponent.h>
#include <Scene/Components/CameraComponent.h>
#include <Scene/Components/MeshComponent.h>
#include <Scene/Components/NameComponent.h>
#include <Scene/Components/AABBComponent.h>

#include <filesystem>

#include <entt/entt.hpp>
#include <DirectXCollision.h>

namespace Engine::Scene::Loader
{
    UniquePtr<SceneObject> SceneLoader::LoadScene(String path, Optional<float32> scale)
    {
        std::filesystem::path filePath = path;
        std::filesystem::path exportPath = filePath;
        exportPath.replace_extension("assbin");

        Assimp::Importer importer;

        const aiScene *aScene;
        // if (std::filesystem::exists(exportPath) && std::filesystem::is_regular_file(exportPath))
        {
            //      aScene = importer.ReadFile(exportPath.string(), 0);
        }
        //  else
        {
            unsigned int preprocessFlags = 0
                //| aiProcess_Triangulate 
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

            if (aScene)
            {
                // Export the preprocessed scene file for faster loading next time.
                // Assimp::Exporter exporter;
                // exporter.Export(aScene, "assbin", exportPath.string(), preprocessFlags);
            }
        }

        auto scene = MakeUnique<SceneObject>();

        LoadingContext context;
        context.RootPath = filePath.parent_path().string();
        context.registry = &scene->GetRegistry();

        context.dataTextures.reserve(aScene->mNumTextures);
        for (uint32 i = 0; i < aScene->mNumTextures; ++i)
        {
            aiTexture *aTexture = aScene->mTextures[i];
            auto texture = GetTexture(aTexture, context);
            context.dataTextures.push_back(texture);
        }

        context.materials.reserve(static_cast<Size>(aScene->mNumMaterials));
        for (uint32 i = 0; i < aScene->mNumMaterials; ++i)
        {
            aiMaterial *aMaterial = aScene->mMaterials[i];
            auto material = ParseMaterial(aMaterial, context);
            context.materials.emplace_back(material);
        }

        context.meshes.reserve(static_cast<Size>(aScene->mNumMeshes));
        for (uint32 i = 0; i < aScene->mNumMeshes; ++i)
        {
            aiMesh *aMesh = aScene->mMeshes[i];
            auto mesh = ParseMesh(aMesh, context);
            context.meshes.push_back(mesh);
        }

        context.lightsMap.reserve(static_cast<Size>(aScene->mNumLights));
        for (uint32 i = 0; i < aScene->mNumLights; ++i)
        {
            aiLight *aLight = aScene->mLights[i];
            context.lightsMap[aLight->mName.C_Str()] = aLight;
        }

        context.camerasMap.reserve(static_cast<Size>(aScene->mNumCameras));
		for (uint32 i = 0; i < aScene->mNumCameras; ++i)
		{
			aiCamera* aCamera = aScene->mCameras[i];
			context.camerasMap[aCamera->mName.C_Str()] = aCamera;
		}

        auto rootEntity = context.registry->create();
        Engine::Scene::Components::RelationshipComponent relationship;
        ParseNode(aScene, aScene->mRootNode, context, rootEntity, &relationship);

        context.registry->emplace<Components::RelationshipComponent>(rootEntity, relationship);
        context.registry->emplace<Components::Root>(rootEntity);

        if (aScene->mNumCameras == 0)
        {
            auto cameraEntity = context.registry->create();
            context.registry->emplace<Components::CameraComponent>(cameraEntity, Camera());
            context.registry->emplace<Components::LocalTransformComponent>(cameraEntity, dx::XMMatrixIdentity());
            
            context.registry->emplace<Components::NameComponent>(cameraEntity, "Default Camera");
            context.registry->emplace<Components::RelationshipComponent>(cameraEntity, Components::RelationshipComponent());
            context.registry->emplace<Components::Root>(cameraEntity);
        }

        std::function<void(const PunctualLight&, const dx::XMMATRIX&, String)> addLight = [&context](const PunctualLight& light, const dx::XMMATRIX& transform, String name)
        {
            auto lightEntity= context.registry->create();
            context.registry->emplace<Components::LightComponent>(lightEntity, light);
            context.registry->emplace<Components::LocalTransformComponent>(lightEntity, transform);
            context.registry->emplace<Components::RelationshipComponent>(lightEntity, Components::RelationshipComponent());
            context.registry->emplace<Components::Root>(lightEntity);
            context.registry->emplace<Components::NameComponent>(lightEntity, name);
        };


        PunctualLight pointLight1;
        pointLight1.SetColor({1.0f, 0.6f, 0.2f});
        pointLight1.SetIntensity(50);
        pointLight1.SetQuadraticAttenuation(1.0f);
        pointLight1.SetEnabled(true);
        pointLight1.SetLightType(LightType::PointLight);

        addLight(pointLight1, DirectX::XMMatrixTranslation(4.0f, 5.0f, -2.0f), "Custom light 1");

        PunctualLight pointLight2;
        pointLight2.SetColor({1.0f, 1.0f, 1.0f});
        pointLight2.SetIntensity(2);
        pointLight2.SetQuadraticAttenuation(1.0f);
        pointLight2.SetEnabled(true);
        pointLight2.SetLightType(LightType::PointLight);

        addLight(pointLight2, DirectX::XMMatrixTranslation(0.0f, 2.0f, 0.0f), "Custom light 2");

        return scene;
    }

    void SceneLoader::ParseNode(const aiScene *aScene, const aiNode *aNode, const LoadingContext &context, entt::entity entity, Engine::Scene::Components::RelationshipComponent* relationship)
    {
        aiVector3D scaling;
		aiQuaternion rotation;
		aiVector3D position;
		aNode->mTransformation.Decompose(scaling, rotation, position);

		auto t = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
		auto s = DirectX::XMMatrixScaling(scaling.x, scaling.y, scaling.z);
		auto r = DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w));

		DirectX::XMFLOAT4X4 transform;
        DirectX::XMMATRIX srt = s * r * t;
		DirectX::XMStoreFloat4x4(&transform, s * r * t);

		if (IsMeshNode(aNode, context))
		{
            context.registry->emplace<Components::NameComponent>(entity, aNode->mName.C_Str());
            context.registry->emplace<Scene::Components::LocalTransformComponent>(entity, srt);

            CreateMeshNode(aNode, context, entity, relationship);
            
		}
        else if (IsLightNode(aNode, context))
        {
            CreateLightNode(aNode, context, entity);

            context.registry->emplace<Components::NameComponent>(entity, "Light_" + std::string(aNode->mName.C_Str()));
            context.registry->emplace<Scene::Components::LocalTransformComponent>(entity, srt);
        }
        else if (IsCameraNode(aNode, context))
        {
            CreateCameraNode(aNode, context, entity);

            context.registry->emplace<Components::NameComponent>(entity, "Camera_" + std::string(aNode->mName.C_Str()));
            context.registry->emplace<Scene::Components::LocalTransformComponent>(entity, srt);
        }
        else
        {
            context.registry->emplace<Components::NameComponent>(entity, aNode->mName.C_Str());
            context.registry->emplace<Scene::Components::LocalTransformComponent>(entity, srt);

            entt::entity nextEntity = aNode->mNumChildren > 0 ? context.registry->create() : entt::null;
            relationship->first = nextEntity;
            relationship->childsCount = aNode->mNumChildren;

            for (uint32 i = 0; i < aNode->mNumChildren; i++)
            {
                auto aChild = aNode->mChildren[i];

                auto childEntity = nextEntity;

                if (i < (aNode->mNumChildren - 1))
                {
                    nextEntity = context.registry->create();
                }
                else
                {
                    nextEntity = entt::null;
                }

                Components::RelationshipComponent childRelationship;
                childRelationship.parent = entity;
                childRelationship.next = nextEntity;
                childRelationship.depth = relationship->depth + 1;

                ParseNode(aScene, aChild, context, childEntity, &childRelationship);

                context.registry->emplace<Components::RelationshipComponent>(childEntity, childRelationship);
            }
        }
    }

    std::tuple<String, Mesh, dx::BoundingBox> SceneLoader::ParseMesh(const aiMesh *aMesh, const LoadingContext &context)
    {
        Mesh mesh;
        mesh.indexBuffer = MakeShared<IndexBuffer>();
        mesh.vertexBuffer = MakeShared<VertexBuffer>();
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
                auto bitnagent = *reinterpret_cast<DirectX::XMFLOAT3 *>(&aMesh->mBitangents[i]);
                float32 symmetry = +1.0f;
                if (DirectX::XMVectorGetX(DirectX::XMVector3Dot(
                        DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&vertex.Normal), DirectX::XMLoadFloat3(&tangent)),
                        DirectX::XMLoadFloat3(&bitnagent))) < 0.0f)
                {
                    symmetry = -1.0f;
                }

                vertex.Tangent = {tangent.x, tangent.y, tangent.z, symmetry};
            }
            vertices.emplace_back(vertex);
        }

        mesh.vertexBuffer->SetData(vertices);
        mesh.vertexBuffer->SetName(StringToWString("Vertices: " + (std::string)(aMesh->mName.C_Str())));

        std::vector<uint16> indices;
        indices.reserve(aMesh->mNumFaces * 3);
        for (uint32 i = 0; i < aMesh->mNumFaces; ++i)
        {
            const auto &face = aMesh->mFaces[i];
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
        }

        mesh.indexBuffer->SetData(indices);
        mesh.indexBuffer->SetName(StringToWString("Indices: " + (std::string)(aMesh->mName.C_Str())));

        mesh.material = context.materials[aMesh->mMaterialIndex];

        switch (aMesh->mPrimitiveTypes)
        {
            case aiPrimitiveType_POINT:
                mesh.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
                break;
            case aiPrimitiveType_LINE:
                mesh.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
                break;
            case aiPrimitiveType_TRIANGLE:
            default:
                mesh.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                break;
        }

        const auto& aabbMin = aMesh->mAABB.mMin;
        const auto& aabbMax = aMesh->mAABB.mMax;

        dx::BoundingBox boundingBox;
        dx::BoundingBox::CreateFromPoints(
            boundingBox, 
            dx::XMVectorSet(aabbMin.x, aabbMin.y, aabbMin.z, 0.0),
            dx::XMVectorSet(aabbMax.x, aabbMax.y, aabbMax.z, 0.0));
        
        return std::make_tuple(aMesh->mName.C_Str(), mesh, boundingBox);
    }

    SharedPtr<Material> SceneLoader::ParseMaterial(const aiMaterial *aMaterial, LoadingContext &context)
    {
        SharedPtr<Material> material = MakeShared<Material>();
        MaterialProperties properties;

        aiString name;
        aMaterial->Get(AI_MATKEY_NAME, name);

        aiColor4D baseColor;
        if (aMaterial->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, baseColor) == aiReturn_SUCCESS)
        {
            properties.baseColor.baseColor = *reinterpret_cast<DirectX::XMFLOAT4 *>(&baseColor);
        }

        float32 metallicFactor = 0;
        if (aMaterial->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, metallicFactor) == aiReturn_SUCCESS)
        {
            properties.metallicRaughness.metallicFactor = metallicFactor;
        }

        float32 roughnessFactor = 0;
        if (aMaterial->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, roughnessFactor) == aiReturn_SUCCESS)
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

        bool unlit = false;
        if (aMaterial->Get(AI_MATKEY_GLTF_UNLIT, unlit) == aiReturn_SUCCESS)
        {
            properties.unlit = unlit;
        }

        bool twoSided;
        if (aMaterial->Get(AI_MATKEY_TWOSIDED, twoSided) == aiReturn_SUCCESS)
        {
            properties.doubleSided = twoSided;
        }

        aiString albedoTexturePath;
        if (aMaterial->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, &albedoTexturePath) == aiReturn_SUCCESS)
        {
            ParseSampler(aMaterial, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE);
            auto albedoTexture = GetTexture(albedoTexturePath, context);
            albedoTexture->SetSRGB(true);
            material->SetBaseColorTexture(albedoTexture);
        }

        aiString normalTexturePath;
        if (aMaterial->GetTexture(aiTextureType_NORMALS, 0, &normalTexturePath) == aiReturn_SUCCESS)
        {
            ParseSampler(aMaterial, aiTextureType_NORMALS, 0);
            auto normalTexture = GetTexture(normalTexturePath, context);
            material->SetNormalTexture(normalTexture);

            float32 scale;
            if (aMaterial->Get(AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_NORMALS, 0), scale) == aiReturn_SUCCESS)
            {
                properties.normalTextureInfo.scale = scale;
            }
        }

        aiString metallicRoughnessTexturePath;
        if (aMaterial->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &metallicRoughnessTexturePath) == aiReturn_SUCCESS)
        {
            ParseSampler(aMaterial, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE);
            auto metallicRoughnessTexture = GetTexture(metallicRoughnessTexturePath, context);
            material->SetMetallicRoughnessTexture(metallicRoughnessTexture);
        }

        aiString ambientOcclusionTexturePath;
        if (aMaterial->GetTexture(aiTextureType_LIGHTMAP, 0, &ambientOcclusionTexturePath) == aiReturn_SUCCESS)
        {
            ParseSampler(aMaterial, aiTextureType_LIGHTMAP, 0);
            auto aoTexture = GetTexture(ambientOcclusionTexturePath, context);
            material->SetAmbientOcclusionTexture(aoTexture);
        }

        aiString emissiveTexturePath;
        if (aMaterial->GetTexture(aiTextureType_EMISSIVE, 0, &emissiveTexturePath) == aiReturn_SUCCESS)
        {
            ParseSampler(aMaterial, aiTextureType_EMISSIVE, 0);
            auto emissiveTexture = GetTexture(emissiveTexturePath, context);
            material->SetEmissiveTexture(emissiveTexture);

            float32 strength;
            if (aMaterial->Get(AI_MATKEY_GLTF_TEXTURE_STRENGTH(aiTextureType_EMISSIVE, 0), strength) == aiReturn_SUCCESS)
            {
                properties.emissive.info.strength = strength;
            }
        }

        material->SetProperties(properties);

        return material;
    }

    void SceneLoader::ParseSampler(const aiMaterial* aMaterial, aiTextureType textureType, unsigned int idx)
    {
        aiTextureMapMode wrapS;
        if (aMaterial->Get<aiTextureMapMode>(AI_MATKEY_MAPPINGMODE_U(textureType, idx), wrapS) == aiReturn_SUCCESS)
        {
        }

        aiTextureMapMode wrapT;
        if (aMaterial->Get<aiTextureMapMode>(AI_MATKEY_MAPPINGMODE_V(textureType, idx), wrapT) == aiReturn_SUCCESS)
        {
        }

        SamplerMagFilter magFilter;
        if (aMaterial->Get<SamplerMagFilter>(AI_MATKEY_GLTF_MAPPINGFILTER_MAG(textureType, idx), magFilter) == aiReturn_SUCCESS)
        {
        }

        SamplerMinFilter minFilter;
        if (aMaterial->Get<SamplerMinFilter>(AI_MATKEY_GLTF_MAPPINGFILTER_MIN(textureType, idx), minFilter) == aiReturn_SUCCESS)
        {
        }
    }

    SharedPtr<Texture> SceneLoader::GetTexture(const aiTexture *aTexture, const LoadingContext &context)
    {
        std::vector<Byte> buffer;
        buffer.resize(aTexture->mWidth);
        memcpy(buffer.data(), aTexture->pcData, aTexture->mWidth);
        String name = context.RootPath + "\\" + aTexture->mFilename.C_Str();
        SharedPtr<Scene::Image> image = Scene::Image::LoadImageFromData(buffer, aTexture->achFormatHint, name);

        SharedPtr<Texture> texture = MakeShared<Texture>(StringToWString(image->GetName()));
        texture->SetImage(image);
        
        return texture;
    }

    SharedPtr<Texture> SceneLoader::GetTexture(const aiString &path, LoadingContext &context)
    {
        if (path.C_Str()[0] != '*')
        {
            std::filesystem::path filePath = context.RootPath + "\\" + path.C_Str();

            String filePathStr = filePath.string();
            auto iter = context.fileTextures.find(filePathStr);
            if (iter != context.fileTextures.end())
            {
                return iter->second;
            }
            else
            {
                if (std::filesystem::exists(filePath))
                {
                    auto image = Scene::Image::LoadImageFromFile(filePathStr);
                    SharedPtr<Texture> texture = MakeShared<Texture>(StringToWString(image->GetName()));
                    texture->SetImage(image);        
                    context.fileTextures[filePathStr] = texture;

                    return texture;
                }
            }
        }
        else
        {
            int index = atoi(path.C_Str() + 1);
            return context.dataTextures[index];
        }

        return nullptr;
    }

	bool SceneLoader::IsLightNode(const aiNode* aNode, const LoadingContext& context)
	{
		auto iter = context.lightsMap.find(aNode->mName.C_Str());
        return iter != context.lightsMap.end();
	}

	bool SceneLoader::IsMeshNode(const aiNode* aNode, const LoadingContext& context)
	{
        return aNode->mNumMeshes > 0;
	}

    bool SceneLoader::IsCameraNode(const aiNode* aNode, const LoadingContext& context)
	{
		auto iter = context.camerasMap.find(aNode->mName.C_Str());
        return iter != context.camerasMap.end();
	}

    void SceneLoader::CreateLightNode(const aiNode* aNode, const LoadingContext &context, entt::entity entity)
    {
		auto iter = context.lightsMap.find(aNode->mName.C_Str());
        aiLight* aLight = iter->second;

        PunctualLight light;

        light.SetEnabled(true);

        switch (aLight->mType)
        {
        case aiLightSource_DIRECTIONAL:
            light.SetLightType(LightType::DirectionalLight);
            break;
        case aiLightSource_POINT:
            light.SetLightType(LightType::PointLight);
            break;
        case aiLightSource_SPOT:
            light.SetLightType(LightType::SpotLight);
            break;
        }

        DirectX::XMFLOAT3 direction{aLight->mDirection.x, aLight->mDirection.z, aLight->mDirection.y};
        light.SetDirection(direction);

        float intensity = max(1.0f, max(aLight->mColorDiffuse.r, max(aLight->mColorDiffuse.g, aLight->mColorDiffuse.b)));
        DirectX::XMFLOAT3 color{aLight->mColorDiffuse.r / intensity, aLight->mColorDiffuse.g / intensity, aLight->mColorDiffuse.b / intensity};
        light.SetColor(color);
        light.SetIntensity(intensity);

        light.SetConstantAttenuation(aLight->mAttenuationConstant);
        light.SetLinearAttenuation(aLight->mAttenuationLinear);
        light.SetQuadraticAttenuation(aLight->mAttenuationQuadratic);

        light.SetInnerConeAngle(aLight->mAngleInnerCone);
        light.SetOuterConeAngle(aLight->mAngleOuterCone);

        Components::LightComponent lightComponent;
        lightComponent.light = light;

        context.registry->emplace<Components::LightComponent>(entity, lightComponent);

    }

    void SceneLoader::CreateMeshNode(const aiNode* aNode, const LoadingContext& context, entt::entity entity, Engine::Scene::Components::RelationshipComponent* relationship)
    {
        entt::entity nextEntity = context.registry->create();;
        relationship->first = nextEntity;
        relationship->childsCount = aNode->mNumMeshes;

		for (uint32 i = 0; i < aNode->mNumMeshes; ++i)
		{
			int32 meshIndex = aNode->mMeshes[i];
			auto meshData = context.meshes[meshIndex];

            auto meshEntity = nextEntity;
            if (i < (aNode->mNumMeshes - 1))
            {
                nextEntity = context.registry->create();
            }
            else
            {
                nextEntity = entt::null;
            }

            context.registry->emplace<Components::LocalTransformComponent>(meshEntity, DirectX::XMMatrixIdentity());

            context.registry->emplace<Components::NameComponent>(meshEntity, std::get<0>(meshData));

            Components::RelationshipComponent meshRelationship;
            meshRelationship.next = nextEntity;
            meshRelationship.parent = entity;
            meshRelationship.depth = relationship->depth + 1;
            context.registry->emplace<Components::RelationshipComponent>(meshEntity, meshRelationship);

            Components::MeshComponent meshComponent;
            meshComponent.mesh = std::get<1>(meshData);

            context.registry->emplace<Components::MeshComponent>(meshEntity, meshComponent);
            context.registry->emplace<Components::AABBComponent>(meshEntity, std::get<2>(meshData));
		}
    }

    void SceneLoader::CreateCameraNode(const aiNode* aNode, const LoadingContext& context, entt::entity entity)
    {
        auto iter = context.camerasMap.find(aNode->mName.C_Str());

        aiCamera* aCamera = iter->second;

        Camera camera;

        camera.SetNearPlane(aCamera->mClipPlaneNear);
        camera.SetFarPlane(aCamera->mClipPlaneFar);
        camera.SetFoV(aCamera->mHorizontalFOV);

        Components::CameraComponent cameraComponent;
        cameraComponent.camera = camera;

        context.registry->emplace<Components::CameraComponent>(entity, cameraComponent);
    }

} // namespace Engine::Scene::Loader