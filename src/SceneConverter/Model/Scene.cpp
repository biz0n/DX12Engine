#include "Scene.h"

#include <spdlog/spdlog.h>

#include <DirectXMesh.h>


#include <queue>
#include <tuple>
#include <cstdio>
#include <span>

namespace SceneConverter::Model
{
    Scene::Scene()
    {
        mStringsStorage.push_back('\0');
        mStringsMap[""] = { 0, 1 };

        Bin3D::Sampler defaultSampler = {};
        defaultSampler.ModeU = Bin3D::Sampler::AddressMode::Mirror;
        defaultSampler.ModeV = Bin3D::Sampler::AddressMode::Mirror;
        defaultSampler.ModeW = Bin3D::Sampler::AddressMode::Mirror;
        defaultSampler.MaxAnisotropy = 16;

        mSamplers.push_back(defaultSampler);
        mSamplersMap[defaultSampler] = 0;
    }

    void Scene::AddRootNode(const Node& node)
    {
        mRootNodes.push_back(node);
    }

    uint32_t Scene::AddMesh(const Bin3D::Mesh& mesh)
    {
        mMeshes.push_back(mesh);
        return mMeshes.size() - 1;
    }

    uint32_t Scene::AddMaterial(const Bin3D::Material& material)
    {
        mMaterials.push_back(material);
        return mMaterials.size() - 1;
    }

    uint32_t Scene::AddLight(const Bin3D::PunctualLight& light)
    {
        mLights.push_back(light);
        return mLights.size() - 1;
    }

    uint32_t Scene::AddCamera(const Bin3D::Camera& camera)
    {
        mCameras.push_back(camera);
        return mCameras.size() - 1;
    }

    uint32_t Scene::AddSampler(const Bin3D::Sampler& sampler)
    {
        auto iter = mSamplersMap.find(sampler);
        if (iter != mSamplersMap.end())
        {
            return iter->second;
        }
        else
        {
            size_t index = mSamplers.size();
            mSamplersMap[sampler] = index;
            mSamplers.push_back(sampler);
            return index;
        }
    }

    Bin3D::DataRegion Scene::AddIndices(const std::vector<uint32_t>& indices)
    {
        Bin3D::DataRegion index = {};
        index.Offset = mIndicesStorage.size();
        index.Size = indices.size();
        mIndicesStorage.insert(mIndicesStorage.end(), indices.begin(), indices.end());

        return index;
    }

    Bin3D::DataRegion Scene::AddVertices(const std::vector<Bin3D::VertexCoordinates>& coordinates, const std::vector<Bin3D::VertexProperties>& properties)
    {
        Bin3D::DataRegion index = {};
        index.Offset = mVerticesCoordinatesStorage.size();
        index.Size = coordinates.size();

        mVerticesCoordinatesStorage.insert(mVerticesCoordinatesStorage.end(), coordinates.begin(), coordinates.end());
        mVerticesPropertiesStorage.insert(mVerticesPropertiesStorage.end(), properties.begin(), properties.end());

        return index;
    }

    Bin3D::DataRegion Scene::AddString(const std::string& str)
    {
        auto it = mStringsMap.find(str);
        if (it != mStringsMap.end())
        {
            return it->second;
        }
        else
        {
            Bin3D::DataRegion index = {};
            index.Offset = mStringsStorage.size();
            index.Size = str.size() + 1;

            const auto* cstr = str.c_str();
            for (size_t i = 0; i < str.size() + 1; ++i)
            {
                mStringsStorage.push_back(cstr[i]);
            }
            
            mStringsMap[str] = index;
            return index;
        }
    }

    uint32_t Scene::AddImage(std::shared_ptr<ImageData> image)
    {
        mImageResources.push_back(image);

        Bin3D::ImagePath imagePath = {};

        mImagePaths.push_back(imagePath);

        return mImageResources.size() - 1;
    }

    void Scene::UnwrapNodeTree()
    {
        mNodes.clear();

        std::queue<std::tuple<Node, size_t>> nodesQueue;
        for (size_t i = 0; i < mRootNodes.size(); ++i)
        {
            nodesQueue.push({ mRootNodes[i], i });
        }

        while (!nodesQueue.empty())
        {
            const auto& [ node, parent ] = nodesQueue.front();

            Bin3D::Node sceneNode = {};
            sceneNode.Type = node.Type;
            switch (sceneNode.Type)
            {
            case Bin3D::Node::NodeType::Camera:
                sceneNode.DataIndex = node.CameraIndex.value_or(0);
                break;
            case Bin3D::Node::NodeType::Light:
                sceneNode.DataIndex = node.LightIndex.value_or(0);
                break;
            case Bin3D::Node::NodeType::Mesh:
                sceneNode.DataIndex = node.MeshIndex.value_or(0);
                break;
            default:
                sceneNode.DataIndex = 0;
                break;
            }
            
            sceneNode.LocalTransform = node.LocalTransform;
            sceneNode.NameIndex = node.NameIndex;

            sceneNode.Parent = parent;
            mNodes.push_back(sceneNode);

            const size_t nextParent = mNodes.size() - 1;

            for (const auto& childNode : node.Children)
            {
                nodesQueue.push({ childNode, nextParent });
            }

            nodesQueue.pop();
        }
    }

    void Scene::FulfillImagePaths()
    {
        mImagePaths.clear();

        for (auto image : mImageResources)
        {
            Bin3D::ImagePath imagePath = {};
            imagePath.PathIndex = AddString(image->GetFileName());

            mImagePaths.push_back(imagePath);
        }
    }
    void Scene::ComputeMeshlets()
    {
        std::vector<DirectX::Meshlet> meshlets;
        std::vector<uint8_t> uniqueVertexIB;
        std::vector<DirectX::MeshletTriangle> primitiveIndices;

        // https://developer.nvidia.com/blog/introduction-turing-mesh-shaders/
        constexpr size_t mesheltMaxVerts = 64;
        constexpr size_t meshletMaxPrimitives = 126;

        for (auto& mesh : mMeshes)
        {
            meshlets.clear();
            uniqueVertexIB.clear();
            primitiveIndices.clear();

            const uint32_t* indices = mIndicesStorage.data() + mesh.Indices.Offset;
            const DirectX::XMFLOAT3* positions = reinterpret_cast<const DirectX::XMFLOAT3*>(mVerticesCoordinatesStorage.data() + mesh.Vertices.Offset);

            auto nFaces = mesh.Indices.Size / 3;
            auto result = DirectX::ComputeMeshlets(
                indices, nFaces,
                positions, mesh.Vertices.Size,
                nullptr,
                meshlets, uniqueVertexIB, primitiveIndices,
                mesheltMaxVerts, meshletMaxPrimitives);

            if (FAILED(result))
            {
                spdlog::error("ComputeMeshlets Failed: {}", result);
            }

            Bin3D::DataRegion meshletsRegion;
            Bin3D::DataRegion primitiveIndicesRegion;
            Bin3D::DataRegion uniqueVertexIndicesRegion;

            meshletsRegion.Offset = mMeshlets.size();
            meshletsRegion.Size = meshlets.size();

            primitiveIndicesRegion.Offset = mPrimitiveIndices.size();
            primitiveIndicesRegion.Size = primitiveIndices.size();

            uniqueVertexIndicesRegion.Offset = mUniqueVertexIndexBuffer.size();
            uniqueVertexIndicesRegion.Size = uniqueVertexIB.size();

            mesh.Meshlets = meshletsRegion;
            mesh.PrimitiveIndices = primitiveIndicesRegion;
            mesh.UniqueVertexIndices = uniqueVertexIndicesRegion;

            for (const auto& meshlet : meshlets)
            {
                mMeshlets.push_back(*reinterpret_cast<const Bin3D::Meshlet*>(&meshlet));
            }

            for (const auto& meshletTriangle : primitiveIndices)
            {
                mPrimitiveIndices.push_back(*reinterpret_cast<const Bin3D::MeshletTriangle*>(&meshletTriangle));
            }

            if (uniqueVertexIB.size() % 4 != 0)
            {
                spdlog::error("Wrong index buffer size: {}", uniqueVertexIB.size());
            }

            mUniqueVertexIndexBuffer.insert(mUniqueVertexIndexBuffer.end(), uniqueVertexIB.begin(), uniqueVertexIB.end());
        }

        auto uniqueVertexIB32 = std::span(reinterpret_cast<uint32_t*>(mUniqueVertexIndexBuffer.data()), mUniqueVertexIndexBuffer.size() / 4);
        auto uniqueVertexIB16 = std::span(reinterpret_cast<uint16_t*>(mUniqueVertexIndexBuffer.data()), mUniqueVertexIndexBuffer.size() / 2);

        mUniqueVertexIB32.insert(mUniqueVertexIB32.end(), uniqueVertexIB32.begin(), uniqueVertexIB32.end());
        mUniqueVertexIB16.insert(mUniqueVertexIB16.end(), uniqueVertexIB16.begin(), uniqueVertexIB16.end());
    }
}
