#include "SceneLoader.h"
#include "Utils.h"
#include "Image.h"

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>

#include <filesystem>

#include <DirectXTex.h>

UniquePtr<SceneObject> SceneLoader::LoadScene(String path, float32 scale)
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
        unsigned int preprocessFlags = //aiProcess_Triangulate |
                                       aiProcess_ConvertToLeftHanded |
                                       //aiProcess_JoinIdenticalVertices |
                                       //aiProcess_GenNormals |
                                       aiProcess_CalcTangentSpace
                                      // aiProcess_OptimizeMeshes |
                                      // aiProcess_OptimizeGraph
                                      ;

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
    context.Scale = scale;

    context.textures.reserve(aScene->mNumTextures);
    for (uint32 i = 0; i < aScene->mNumTextures; ++i)
    {
        aiTexture *aTexture = aScene->mTextures[i];
        auto texture = GetTexture(aTexture, context);
        context.textures.push_back(texture);
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
        aiMesh* aMesh = aScene->mMeshes[i];
        auto mesh = ParseMesh(aMesh, context);
        context.meshes.push_back(mesh);
    }

    ParseNode(aScene, aScene->mRootNode, scene.get(), nullptr, context);


    std::vector<LightUniform> lights;
    lights.reserve(static_cast<Size>(aScene->mNumLights));
    for (uint32 i = 0; i < aScene->mNumLights; ++i)
    {
        aiLight* aLight = aScene->mLights[i];

        LightUniform light = {};
        light.Enabled = true;

        switch (aLight->mType)
        {
            case aiLightSource_DIRECTIONAL:
                light.LightType = DIRECTIONAL_LIGHT;
                light.Enabled = false;
            break;
            case aiLightSource_POINT:
                light.LightType = POINT_LIGHT;
                light.Enabled = false;
            break;
            case aiLightSource_SPOT:
                light.LightType = SPOT_LIGHT;
                //light.Enabled = false;
            break;
        }

        light.PositionWS = *reinterpret_cast<DirectX::XMFLOAT3*>(&aLight->mPosition);
        light.DirectionWS = *reinterpret_cast<DirectX::XMFLOAT3*>(&aLight->mDirection);
        
        for (uint32 j = 0; j < aScene->mRootNode->mNumChildren; ++j)
        {
            if (aScene->mRootNode->mChildren[j]->mName == aLight->mName)
            {
                auto transformation = &aScene->mRootNode->mChildren[j]->mTransformation;
                
                aiVector3D scaling;
                aiQuaternion rotation;
                aiVector3D position;
                transformation->Decompose(scaling, rotation, position);

                auto t = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
                auto s = DirectX::XMMatrixScaling(scaling.x, scaling.y, scaling.z);
                DirectX::XMFLOAT4 quaternion = {rotation.x, rotation.y, rotation.z, rotation.w};
                auto r = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&quaternion));

                DirectX::XMFLOAT4 dir {0.0f, 0.0f, -1.0f, 0.0f};
                DirectX::XMStoreFloat3(
                    &light.DirectionWS, 
                    DirectX::XMVector3Normalize(
                        DirectX::XMVector4Transform(
                            DirectX::XMLoadFloat4(&dir),
                            t * r * s
                            )));

                DirectX::XMFLOAT4 pos {0.0f, 0.0f, 0.0f, 1.0f};
                DirectX::XMStoreFloat3(
                    &light.PositionWS,
                    DirectX::XMVector4Transform(
                        DirectX::XMLoadFloat4(&pos),
                        t * r * s
                        ));
                break;
            }
        }
        
        light.Color = *reinterpret_cast<DirectX::XMFLOAT3*>(&aLight->mColorDiffuse);

        light.ConstantAttenuation = aLight->mAttenuationConstant;
        light.LinearAttenuation = aLight->mAttenuationLinear;
        light.QuadraticAttenuation = aLight->mAttenuationQuadratic;

        light.InnerConeAngle = aLight->mAngleInnerCone;
        light.OuterConeAngle = aLight->mAngleOuterCone;

        lights.emplace_back(light);
    }


    LightUniform directionalLight = {};
    directionalLight.Color = {20.0f, 20.0f, 20.0f};
    directionalLight.DirectionWS = {0.2f, -1.0f, 0.2f};
    directionalLight.Enabled = false;
    directionalLight.LightType = DIRECTIONAL_LIGHT;
    //lights.emplace_back(directionalLight);

    /*LightUniform spotLight = {};
    spotLight.Color = {20.0f, 20.0f, 20.0f};
    auto target = mCamera.GetTarget();
    spotLight.DirectionWS = target;
    spotLight.PositionWS = cb.EyePos; 
    spotLight.InnerConeAngle = DirectX::XMConvertToRadians(10.0f);
    spotLight.OuterConeAngle = DirectX::XMConvertToRadians(14.0f);

    spotLight.ConstantAttenuation = 0.0f;
    spotLight.LinearAttenuation = 0.00f;
    spotLight.QuadraticAttenuation = 1.0f;
    spotLight.Enabled = false;
    spotLight.LightType = SPOT_LIGHT;
    lights.emplace_back(spotLight);*/

    LightUniform pointLight = {};
    pointLight.Color = {50.0f, 30.0f, 10.0f};
    pointLight.PositionWS = {4.0f, 5.0f, -2.0f};


    pointLight.QuadraticAttenuation = 1.0f;
    pointLight.Enabled = true;
    pointLight.LightType = POINT_LIGHT;
    //lights.emplace_back(pointLight);

    LightUniform pointLight2 = {};
    pointLight2.Color = {20.0f, 20.0f, 20.0f};
    pointLight2.PositionWS = {00.0f, 2.0f, 0.0f};


    pointLight2.QuadraticAttenuation = 1.0f;
    pointLight2.Enabled = true;
    pointLight2.LightType = POINT_LIGHT;
   lights.emplace_back(pointLight2);
    

    scene->lights = std::move(lights);

    return scene;
}

void SceneLoader::ParseNode(const aiScene *aScene, aiNode *aNode, SceneObject *scene, Node *parentNode, const LoadingContext& context)
{
    UniquePtr<Node> node = MakeUnique<Node>();

    node->mParent = parentNode;
    node->LocalTransform = *reinterpret_cast<DirectX::XMFLOAT4X4 *>(&aNode->mTransformation);

    aiVector3D scaling;
    aiQuaternion rotation;
    aiVector3D position;
    aNode->mTransformation.Decompose(scaling, rotation, position);

    auto t = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
    auto s = DirectX::XMMatrixScaling(scaling.x, scaling.y, scaling.z);
    DirectX::XMFLOAT4 quaternion = {rotation.x, rotation.y, rotation.z, rotation.w};
    auto r = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&quaternion));

    DirectX::XMStoreFloat4x4(&node->LocalTransform, t * r * s );

    for (uint32 i = 0; i < aNode->mNumMeshes; ++i)
    {
        int32 meshIndex = aNode->mMeshes[i];
        auto mesh = context.meshes[meshIndex];
        node->mMeshes.push_back(mesh);
    }

    auto nodePtr = node.get();
    scene->nodes.push_back(std::move(node));

    for (uint32 i = 0; i < aNode->mNumChildren; i++)
    {
        auto aChild = aNode->mChildren[i];
        if (aChild->mNumMeshes == 0 && aChild->mNumChildren == 0)
        {
            continue;
        }

        ParseNode(aScene, aChild, scene, nodePtr, context);
    }
}

SharedPtr<Mesh> SceneLoader::ParseMesh(const aiMesh *aMesh, const LoadingContext& context)
{
    SharedPtr<Mesh> mesh = MakeShared<Mesh>();
    std::vector<Vertex> vertices;
    vertices.reserve(aMesh->mNumVertices);
    for (uint32 i = 0; i < aMesh->mNumVertices; ++i)
    {
        Vertex vertex = {};
        vertex.Vertex.x = aMesh->mVertices[i].x * context.Scale;
        vertex.Vertex.y = aMesh->mVertices[i].y * context.Scale;
        vertex.Vertex.z = aMesh->mVertices[i].z * context.Scale;
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
            auto tangent = *reinterpret_cast<DirectX::XMFLOAT3*>(&aMesh->mTangents[i]);
            auto bitnagent = *reinterpret_cast<DirectX::XMFLOAT3*>(&aMesh->mBitangents[i]);
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

    mesh->mVertexBuffer.SetData(vertices);

    std::vector<uint16> indices;
    indices.reserve(aMesh->mNumFaces * 3);
    for (uint32 i = 0; i < aMesh->mNumFaces; ++i)
    {
        const auto &face = aMesh->mFaces[i];
        indices.push_back(face.mIndices[0]);
        indices.push_back(face.mIndices[1]);
        indices.push_back(face.mIndices[2]);
    }

    mesh->mIndexBuffer.SetData(indices);

    mesh->material = context.materials[aMesh->mMaterialIndex];

    return mesh;
}

SharedPtr<Material> SceneLoader::ParseMaterial(const aiMaterial *aMaterial, LoadingContext& context)
{
    SharedPtr<Material> material = MakeShared<Material>();
    MaterialProperties properties;

    aiString name;
    aMaterial->Get(AI_MATKEY_NAME, name);

    aiColor4D baseColor;
    if (aMaterial->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, baseColor) == aiReturn_SUCCESS)
    {
        properties.albedo.baseColor = *reinterpret_cast<DirectX::XMFLOAT4*>(&baseColor);
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
    if (aMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveFactor))
    {
        properties.emissive.factor = *reinterpret_cast<DirectX::XMFLOAT3*>(&emissiveFactor);
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
    if (aMaterial->Get(AI_MATKEY_GLTF_UNLIT, unlit))
    {
        properties.unlit = unlit;
    }

    bool twoSided;
    if (aMaterial->Get(AI_MATKEY_TWOSIDED, twoSided))
    {
        properties.doubleSided = twoSided;
    }

    aiString albedoTexturePath;
    if (aMaterial->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, &albedoTexturePath) == aiReturn_SUCCESS)
    {
        auto albedoTexture = GetTexture(albedoTexturePath, context);
        albedoTexture->SetSRGB(true);
        material->SetAlbedoTexture(albedoTexture);
    }

    aiString normalTexturePath;
    if (aMaterial->GetTexture(aiTextureType_NORMALS, 0, &normalTexturePath) == aiReturn_SUCCESS)
    {
        auto normalTexture = GetTexture(normalTexturePath, context);
        material->SetNormalTexture(normalTexture);

        float32 scale;
        if (aMaterial->Get(AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_NORMALS, 0), scale))
        {
            properties.normalTextureInfo.scale = scale;
        }
    }

    aiString metallicRoughnessTexturePath;
    if (aMaterial->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &metallicRoughnessTexturePath) == aiReturn_SUCCESS)
    {
        auto metallicRoughnessTexture = GetTexture(metallicRoughnessTexturePath, context);
        material->SetMetallicRoughnessTexture(metallicRoughnessTexture);
    }

    aiString ambientOcclusionTexturePath;
    if (aMaterial->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &ambientOcclusionTexturePath) == aiReturn_SUCCESS)
    {
        auto aoTexture = GetTexture(ambientOcclusionTexturePath, context);
        material->SetAmbientOcclusionTexture(aoTexture);
    }

    aiString emissiveTexturePath;
    if (aMaterial->GetTexture(aiTextureType_EMISSIVE, 0, &emissiveTexturePath) == aiReturn_SUCCESS)
    {
        auto emissiveTexture = GetTexture(emissiveTexturePath, context);
        material->SetEmissiveTexture(emissiveTexture);

        float32 strength;
        if (aMaterial->Get(AI_MATKEY_GLTF_TEXTURE_STRENGTH(aiTextureType_EMISSIVE, 0), strength))
        {
            properties.emissive.info.strength =strength;
        }
    }

    material->SetProperties(properties);

    return material;
}

SharedPtr<Texture> SceneLoader::GetTexture(const aiTexture* aTexture, const LoadingContext& context)
{
    std::vector<Byte> buffer;
    buffer.resize(aTexture->mWidth);
    memcpy(buffer.data(), aTexture->pcData, aTexture->mWidth);
    String name = context.RootPath + "\\" + aTexture->mFilename.C_Str();
    SharedPtr<Image> image = Image::LoadImageFromData(buffer, aTexture->achFormatHint, name);

    SharedPtr<Texture> texture = MakeShared<Texture>(Utils::ToWide(image->GetName()));
    texture->SetImage(image);
    return texture;
}

SharedPtr<Texture> SceneLoader::GetTexture(const aiString &path, LoadingContext& context)
{
    if (path.C_Str()[0] != '*')
    {
        std::filesystem::path filePath = context.RootPath + "\\" + path.C_Str();

        SharedPtr<Image> image;
        String filePathStr = filePath.string();
        auto iter = context.images.find(filePathStr);
        if (iter != context.images.end())
        {
            image = iter->second;
        }
        else
        {
            if (std::filesystem::exists(filePath))
            {
                image = Image::LoadImageFromFile(filePathStr);
                context.images[filePathStr] = image;
            }
        }

        SharedPtr<Texture> texture = MakeShared<Texture>(Utils::ToWide(image->GetName()));
        texture->SetImage(image);
        return texture;
        
    }
    else
    {
        int index = atoi(path.C_Str() + 1);
        return context.textures[index];
    }

    return nullptr;
}