#include "SceneImporter.h"

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/GltfMaterial.h>


#include <filesystem>
#include <numbers>

#include <DirectXCollision.h>

using namespace Bin3D;

namespace SceneConverter::Importer
{
    //! Values for the Sampler::magFilter field
    enum class GltfSamplerMagFilter : unsigned int
    {
        Unset = 0,
        Nearest = 9728,
        Linear = 9729
    };

    //! Values for the Sampler::minFilter field
    enum class GltfSamplerMinFilter : unsigned int
    {
        Unset = 0,
        Nearest = 9728,
        Linear = 9729,
        Nearest_Mipmap_Nearest = 9984,
        Linear_Mipmap_Nearest = 9985,
        Nearest_Mipmap_Linear = 9986,
        Linear_Mipmap_Linear = 9987
    };

    Model::Scene SceneImporter::LoadScene(const std::string& path, std::optional<float> scale)
    {
        std::filesystem::path filePath = path;

        Assimp::Importer importer;

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

        const aiScene* aScene = importer.ReadFile(filePath.string(), preprocessFlags);

        LoadingContext context = {};
        context.RootPath = filePath.parent_path().string();
        context.Scene = {};

        for (unsigned int i = 0; i < aScene->mNumTextures; ++i)
        {
            aiTexture *aTexture = aScene->mTextures[i];
            auto image = ParseImage(aTexture, context);
            context.Scene.AddImage(image);
        }

        for (unsigned int i = 0; i < aScene->mNumMaterials; ++i)
        {
            aiMaterial *aMaterial = aScene->mMaterials[i];
            auto material = ParseMaterial(aMaterial, i, context);
            context.Scene.AddMaterial(material);
        }

        for (unsigned int i = 0; i < aScene->mNumMeshes; ++i)
        {
            aiMesh *aMesh = aScene->mMeshes[i];
            auto mesh = ParseMesh(aMesh, context);
            context.Scene.AddMesh(mesh);
        }

        for (unsigned int i = 0; i < aScene->mNumLights; ++i)
        {
            aiLight *aLight = aScene->mLights[i];
            context.LightsIndexMap[aLight->mName.C_Str()] = context.Scene.AddLight(ParseLight(aLight));
        }

        for (unsigned int i = 0; i < aScene->mNumCameras; ++i)
        {
            aiCamera* aCamera = aScene->mCameras[i];
            context.CamerasIndexMap[aCamera->mName.C_Str()] = context.Scene.AddCamera(ParseCamera(aCamera));
        }

        const auto rootNode = ParseNode(aScene->mRootNode, aScene, context);
        context.Scene.AddRootNode(rootNode);

        if (aScene->mNumCameras == 0)
        {
            Camera camera;
            camera.Type = CameraType::Perspective;
            camera.NearPlane = 0.001f;
            camera.FarPlane = 100.0f;
            camera.FoV = 45 * std::numbers::pi_v<float> / 180.0f;

            Model::Node cameraNode;
            cameraNode.CameraIndex = context.Scene.AddCamera(camera);
            DirectX::XMStoreFloat4x4(&cameraNode.LocalTransform, DirectX::XMMatrixIdentity());
            context.Scene.AddCamera(camera);
            context.Scene.AddRootNode(cameraNode);
        }

        std::function<void(const PunctualLight&, const DirectX::XMMATRIX&)> addLight = [&context](const PunctualLight& light, const DirectX::XMMATRIX& transform)
        {
            Model::Node lightNode;
            DirectX::XMStoreFloat4x4(&lightNode.LocalTransform, transform);
            lightNode.LightIndex = context.Scene.AddLight(light);
            context.Scene.AddRootNode(lightNode);
        };


        PunctualLight light1 = {};
        light1.Color = {1.0f, 0.6f, 0.2f};
        light1.Intensity = 50;
        light1.ConstantAttenuation = 0;
        light1.LinearAttenuation = 0;
        light1.QuadraticAttenuation = 1;
        light1.InnerConeAngle = 0;
        light1.OuterConeAngle = 0;
        light1.LightType = LightType::PointLight;

       // addLight(light1, DirectX::XMMatrixTranslation(4.0f, 5.0f, -2.0f));

        PunctualLight light2 = {};
        light2.Color = {1.0f, 1.0f, 1.0f};
        light2.Intensity = 2;
        light2.ConstantAttenuation = 0;
        light2.LinearAttenuation = 0;
        light2.QuadraticAttenuation = 1;
        light2.InnerConeAngle = 0;
        light2.OuterConeAngle = 0;
        light2.LightType = LightType::DirectionalLight;

       // addLight(light2, DirectX::XMMatrixTranslation(0.0f, 2.0f, 0.0f));

        context.Scene.UnwrapNodeTree();

        return context.Scene;
    }

    Model::Node SceneImporter::ParseNode(const aiNode *aNode, const aiScene* aScene, LoadingContext &context)
    {
        aiVector3D scaling;
        aiQuaternion rotation;
        aiVector3D position;
        aNode->mTransformation.Decompose(scaling, rotation, position);

        auto t = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
        auto s = DirectX::XMMatrixScaling(scaling.x, scaling.y, scaling.z);
        auto r = DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w));

        DirectX::XMMATRIX srt = s * r * t; 

        std::string name = aNode->mName.C_Str();
        Model::Node node = {};


        std::string nodeName;
        if (IsMeshNode(aNode, context))
        {
            nodeName = "Mesh_" + name;
            
            node.Type = Node::NodeType::Mesh;
            DirectX::XMStoreFloat4x4(&node.LocalTransform, srt);
            std::vector<uint32_t> meshIndices;
            for (unsigned int i = 0; i < aNode->mNumMeshes; ++i)
            {
                unsigned int meshIndex = aNode->mMeshes[i];
                meshIndices.push_back(meshIndex);
                node.MeshIndices = context.Scene.AddNodeMeshIndices(meshIndices);
            }
        }
        else if (IsLightNode(aNode, context))
        {
            nodeName = "Light_" + name;

            node.Type = Node::NodeType::Light;
            node.LightIndex = context.LightsIndexMap[name];
            const auto* light = aScene->mLights[*node.LightIndex];
            DirectX::XMStoreFloat4x4(&node.LocalTransform, LightMatrixFix(light, srt));
        }
        else if (IsCameraNode(aNode, context))
        {
            nodeName = "Camera_" + name;

            node.Type = Node::NodeType::Camera;
            DirectX::XMStoreFloat4x4(&node.LocalTransform, srt);
            node.CameraIndex = context.CamerasIndexMap[name];
        }
        else
        {
            nodeName = name;
            node.Type = Node::NodeType::Node;
            DirectX::XMStoreFloat4x4(&node.LocalTransform, srt);

            for (size_t i = 0; i < aNode->mNumChildren; i++)
            {
                auto aChild = aNode->mChildren[i];
                Model::Node child = ParseNode(aChild, aScene, context);
                node.Children.push_back(child);
            }
        }

        node.NameIndex = context.Scene.AddString(nodeName);

        return node;
    }

    size_t SceneImporter::ParseSampler(const aiMaterial* aMaterial, aiTextureType textureType, unsigned int idx, LoadingContext& context)
    {
        Sampler sampler = context.Scene.GetDefaultSampler();

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

        GltfSamplerMagFilter magFilter;
        if (aMaterial->Get<GltfSamplerMagFilter>(AI_MATKEY_GLTF_MAPPINGFILTER_MAG(textureType, idx), magFilter) == aiReturn_SUCCESS)
        {
            switch (magFilter)
            {
                case GltfSamplerMagFilter::Nearest:
                    sampler.MagFilter = Sampler::Filter::Point;
                    break;
                case GltfSamplerMagFilter::Linear:
                    sampler.MagFilter = Sampler::Filter::Linear;
                    break;
            }
        }

        GltfSamplerMinFilter minFilter;
        if (aMaterial->Get<GltfSamplerMinFilter>(AI_MATKEY_GLTF_MAPPINGFILTER_MIN(textureType, idx), minFilter) == aiReturn_SUCCESS)
        {
            switch (minFilter)
            {
                case GltfSamplerMinFilter::Nearest:
                    sampler.MinFilter = Sampler::Filter::Point;
                    sampler.MipFilter = Sampler::Filter::Point;
                    sampler.MinLod = 0;
                    sampler.MaxLod = 0.25;
                    break;
                case GltfSamplerMinFilter::Linear:
                    sampler.MinFilter = Sampler::Filter::Linear;
                    sampler.MipFilter = Sampler::Filter::Linear;
                    sampler.MinLod = 0;
                    sampler.MaxLod = 0.25;
                    break;
                case GltfSamplerMinFilter::Nearest_Mipmap_Nearest:
                    sampler.MinFilter = Sampler::Filter::Point;
                    sampler.MipFilter = Sampler::Filter::Point;
                    break;
                case GltfSamplerMinFilter::Linear_Mipmap_Nearest:
                    sampler.MinFilter = Sampler::Filter::Linear;
                    sampler.MipFilter = Sampler::Filter::Point;
                    break;
                case GltfSamplerMinFilter::Nearest_Mipmap_Linear:
                    sampler.MinFilter = Sampler::Filter::Point;
                    sampler.MipFilter = Sampler::Filter::Linear;
                    break;
                case GltfSamplerMinFilter::Linear_Mipmap_Linear:
                    sampler.MinFilter = Sampler::Filter::Linear;
                    sampler.MipFilter = Sampler::Filter::Linear;
                    break;
            }
        }

        return context.Scene.AddSampler(sampler);
    }

    std::shared_ptr<Model::ImageData> SceneImporter::ParseImage(const aiTexture *aTexture, const LoadingContext &context)
    {
        std::vector<byte> buffer;
        buffer.resize(aTexture->mWidth);
        memcpy(buffer.data(), aTexture->pcData, aTexture->mWidth);
        std::string name = context.RootPath + "\\" + aTexture->mFilename.C_Str();
        std::shared_ptr<Model::ImageData> image = Model::ImageData::LoadImageFromData(buffer, aTexture->achFormatHint, name);

        return image;
    }

    size_t SceneImporter::GetImage(const aiString &path, LoadingContext &context, unsigned int matIdx, Model::TextureUsage usage)
    {
        size_t index = 0;

        if (path.C_Str()[0] != '*')
        {
            std::filesystem::path filePath = context.RootPath + "\\" + path.C_Str();

            std::string filePathStr = filePath.string();
            auto iter = context.ImagesIndexMap.find(filePathStr);
            if (iter != context.ImagesIndexMap.end())
            {
                index = iter->second;
            }
            else
            {
                if (std::filesystem::exists(filePath))
                {
                    auto image = Model::ImageData::LoadImageFromFile(filePathStr);
                    index = context.Scene.AddImage(image);
                    context.ImagesIndexMap[filePathStr] = index;
                }
            }
        }
        else
        {
            index = atoi(path.C_Str() + 1) + 1;
        }

        if (index > 0)
        {
            context.Scene.GetImageResources()[index]->SetUseAs(usage);
            context.Scene.GetImageResources()[index]->SetMaterialId(matIdx);
        }

        return index;
    }

    bool SceneImporter::IsLightNode(const aiNode* aNode, const LoadingContext& context)
    {
        auto iter = context.LightsIndexMap.find(aNode->mName.C_Str());
        return iter != context.LightsIndexMap.end();
    }

    bool SceneImporter::IsMeshNode(const aiNode* aNode, const LoadingContext& context)
    {
        return aNode->mNumMeshes > 0;
    }

    bool SceneImporter::IsCameraNode(const aiNode* aNode, const LoadingContext& context)
    {
        auto iter = context.CamerasIndexMap.find(aNode->mName.C_Str());
        return iter != context.CamerasIndexMap.end();
    }

    Camera SceneImporter::ParseCamera(const aiCamera* aCamera)
    {
        Camera camera = {};

        camera.FarPlane = aCamera->mClipPlaneFar;
        camera.NearPlane = aCamera->mClipPlaneNear;

        if (aCamera->mHorizontalFOV != 0.0f)
        {
            camera.Type = CameraType::Perspective;
            camera.FoV = aCamera->mHorizontalFOV;
        }
        else
        {
            camera.Type = CameraType::Orthographic;
            camera.OrthographicXMag = aCamera->mOrthographicWidth;
            camera.OrthographicYMag = aCamera->mOrthographicWidth / aCamera->mAspect;
        }

        return camera;
    }

    PunctualLight SceneImporter::ParseLight(const aiLight* aLight)
    {
        PunctualLight light = {};

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

    Mesh SceneImporter::ParseMesh(const aiMesh* aMesh, LoadingContext& context)
    {
        Mesh mesh = {};
        
        std::string meshName = aMesh->mName.C_Str();
        mesh.NameIndex = context.Scene.AddString(meshName);

        std::vector<Vertex> vertices;
        vertices.reserve(aMesh->mNumVertices);

        for (unsigned int i = 0; i < aMesh->mNumVertices; ++i)
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
                float symmetry = +1.0f;
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

        mesh.Vertices = context.Scene.AddVertices(vertices);

        std::vector<uint32_t> indices;
        indices.reserve(aMesh->mNumFaces * 3);
        for (unsigned int i = 0; i < aMesh->mNumFaces; ++i)
        {
            const auto &face = aMesh->mFaces[i];
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
        }

        mesh.Indices = context.Scene.AddIndices(indices);

        mesh.MaterialIndex = aMesh->mMaterialIndex;

        const auto& aabbMin = aMesh->mAABB.mMin;
        const auto& aabbMax = aMesh->mAABB.mMax;

        DirectX::BoundingBox boundingBox;
        DirectX::BoundingBox::CreateFromPoints(
                boundingBox,
                DirectX::XMVectorSet(aabbMin.x, aabbMin.y, aabbMin.z, 0.0),
                DirectX::XMVectorSet(aabbMax.x, aabbMax.y, aabbMax.z, 0.0));

        mesh.AABB = boundingBox;

        return mesh;
    }

    Material SceneImporter::ParseMaterial(const aiMaterial* aMaterial, unsigned int index, LoadingContext& context)
    {
        Material material = {};
        MaterialProperties properties = {};

        aiString name;
        aMaterial->Get(AI_MATKEY_NAME, name);

        aiColor4D baseColor;
        if (aMaterial->Get(AI_MATKEY_BASE_COLOR, baseColor) == aiReturn_SUCCESS)
        {
            properties.baseColor.baseColor = *reinterpret_cast<DirectX::XMFLOAT4 *>(&baseColor);
        }

        float metallicFactor = 0;
        if (aMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor) == aiReturn_SUCCESS)
        {
            properties.metallicRaughness.metallicFactor = metallicFactor;
        }

        float roughnessFactor = 0;
        if (aMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor) == aiReturn_SUCCESS)
        {
            properties.metallicRaughness.roughnessFactor = roughnessFactor;
        }

        aiColor3D emissiveFactor;
        if (aMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveFactor) == aiReturn_SUCCESS)
        {
            properties.emissive.factor = *reinterpret_cast<DirectX::XMFLOAT3 *>(&emissiveFactor);
        }

        float alphaCutoff = 0.5f;
        if (aMaterial->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCutoff) == aiReturn_SUCCESS)
        {
            properties.alphaCutoff = alphaCutoff;
        }

        aiString alphaMode;
        if (aMaterial->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode) == aiReturn_SUCCESS)
        {
            auto alphaModeStr = std::string(alphaMode.C_Str());
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
            material.BaseColorTextureIndex = GetImage(albedoTexturePath, context, index, Model::TextureUsage::BaseColor);
        }

        aiString normalTexturePath;
        if (aMaterial->GetTexture(aiTextureType_NORMALS, 0, &normalTexturePath) == aiReturn_SUCCESS)
        {
            material.NormalTextureSamplerIndex = ParseSampler(aMaterial, aiTextureType_NORMALS, 0, context);
            material.NormalTextureIndex = GetImage(normalTexturePath, context, index, Model::TextureUsage::Normal);

            float scale;
            if (aMaterial->Get(AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_NORMALS, 0), scale) == aiReturn_SUCCESS)
            {
                properties.normalTextureInfo.scale = scale;
            }
        }

        aiString metallicRoughnessTexturePath;
        if (aMaterial->GetTexture(aiTextureType_METALNESS, 0, &metallicRoughnessTexturePath) == aiReturn_SUCCESS)
        {
            material.MetallicRoughnessTextureSamplerIndex = ParseSampler(aMaterial, aiTextureType_METALNESS, 0, context);
            material.MetallicRoughnessTextureIndex = GetImage(metallicRoughnessTexturePath, context, index, Model::TextureUsage::MetallicRoughness);
        }

        aiString ambientOcclusionTexturePath;
        if (aMaterial->GetTexture(aiTextureType_LIGHTMAP, 0, &ambientOcclusionTexturePath) == aiReturn_SUCCESS)
        {
            material.AmbientOcclusionTextureSamplerIndex = ParseSampler(aMaterial, aiTextureType_LIGHTMAP, 0, context);
            material.AmbientOcclusionTextureIndex = GetImage(ambientOcclusionTexturePath, context, index, Model::TextureUsage::AmbientOcclusion);
        }

        aiString emissiveTexturePath;
        if (aMaterial->GetTexture(aiTextureType_EMISSIVE, 0, &emissiveTexturePath) == aiReturn_SUCCESS)
        {
            material.EmissiveTextureSamplerIndex = ParseSampler(aMaterial, aiTextureType_EMISSIVE, 0, context);
            material.EmissiveTextureIndex = GetImage(emissiveTexturePath, context, index, Model::TextureUsage::Emissive);

            float strength;
            if (aMaterial->Get(AI_MATKEY_GLTF_TEXTURE_STRENGTH(aiTextureType_EMISSIVE, 0), strength) == aiReturn_SUCCESS)
            {
                properties.emissive.info.strength = strength;
            }
        }

        material.MaterialProperties = properties;

        return material;
    }

    DirectX::XMMATRIX SceneImporter::LightMatrixFix(const aiLight* aLight, DirectX::XMMATRIX srt)
    {
        DirectX::XMVECTOR direction = DirectX::XMVectorSet(aLight->mDirection.x, aLight->mDirection.z, aLight->mDirection.y, 0.f );
        DirectX::XMVECTOR up = DirectX::XMVectorSet( aLight->mUp.x, aLight->mUp.z, aLight->mUp.y, 0.f );

        if (aLight->mType != aiLightSource_POINT)
        {
            auto local = DirectX::XMMatrixLookToLH(DirectX::XMVectorZero(), direction, up);

            DirectX::XMVECTOR d;
            auto inv = DirectX::XMMatrixInverse(&d, local);

            return DirectX::XMMatrixMultiply(inv, srt);
        }
        else
        {
            return srt;
        }
    }

} // namespace Engine::Scene::Loader