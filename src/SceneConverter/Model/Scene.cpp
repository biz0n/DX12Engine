#include "Scene.h"

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif


#include <spdlog/spdlog.h>

#include <Model/MeshProcessor.h>

#include <queue>
#include <tuple>
#include <cstdio>
#include <span>
#include <numbers>

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

    uint32_t Scene::AddMesh(const RawMesh& mesh)
    {
        mRawMeshes.push_back(mesh);
        return mRawMeshes.size() - 1;
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
        return AddIndices(reinterpret_cast<const uint8_t*>(indices.data()), indices.size() * sizeof(uint32_t));
    }

    Bin3D::DataRegion Scene::AddIndices(const std::vector<uint16_t>& indices)
    {
        auto region =  AddIndices(reinterpret_cast<const uint8_t*>(indices.data()), indices.size() * sizeof(uint16_t));
        
        // round up to uint32 sizes
        if (indices.size() % 2 == 1)
        {
            mIndicesStorage.push_back(0);
            mIndicesStorage.push_back(0);
        }

        return region;
    }

    Bin3D::DataRegion Scene::AddIndices(const uint8_t* indices, uint32_t count)
    {
        Bin3D::DataRegion index = {};
        index.Offset = mIndicesStorage.size();
        index.Size = count;

        mIndicesStorage.resize(index.Offset + index.Size);

        memcpy(mIndicesStorage.data() + index.Offset, indices, count);

        return index;
    }

    Bin3D::DataRegion Scene::AddVertices(const std::vector<RawVertex>& vertices)
    {
        Bin3D::DataRegion index = {};
        index.Offset = mVerticesCoordinatesStorage.size();
        index.Size = vertices.size();

        for (const auto& vertex : vertices)
        {
            mVerticesCoordinatesStorage.push_back(vertex.GetVertexCoordinates());
            mVerticesPropertiesStorage.push_back(vertex.GetVertexProperties());
        }

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

    std::string Scene::GetString(const Bin3D::DataRegion& region)
    {
        return std::string(mStringsStorage.begin() + region.Offset, mStringsStorage.begin() + region.Offset + region.Size);
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
    void Scene::ProcessMeshes()
    {
        MeshProcessor processor = {};

        for (const auto& node : mNodes)
        {
            if (node.Type != Bin3D::Node::NodeType::Mesh)
            {
                continue;
            }

            std::string meshName = GetString(node.NameIndex);
            const auto& rawMesh = mRawMeshes[node.DataIndex];

            spdlog::info("Process `{}` mesh #{}", meshName, node.DataIndex);

            auto meshletsData = processor.ProcessMesh(rawMesh);

            spdlog::info("Process `{}` mesh #{} completed\n", meshName, node.DataIndex);

            Bin3D::Mesh mesh = {};

            auto verticesCount = rawMesh.Vertices.size();
            auto indicesCount = rawMesh.Indices.size();

            if (verticesCount <= std::numeric_limits<uint16_t>::max())
            {
                std::vector<uint16_t> indices;
                indices.reserve(indicesCount);
                for (size_t i = 0; i < indicesCount; ++i)
                {
                    indices.push_back(rawMesh.Indices[i]);
                }
                mesh.IndexSize = sizeof(uint16_t);
                mesh.Indices = AddIndices(indices);
            }
            else
            {
                std::vector<uint32_t> indices;
                indices.reserve(indicesCount);
                for (size_t i = 0; i < indicesCount; ++i)
                {
                    indices.push_back(rawMesh.Indices[i]);
                }
                mesh.IndexSize = sizeof(uint32_t);
                mesh.Indices = AddIndices(indices);
            }

            mesh.Vertices = AddVertices(rawMesh.Vertices);
            mesh.AABB = rawMesh.AABB;
            mesh.MaterialIndex = rawMesh.MaterialIndex;

            mesh.Meshlets.Offset = mMeshlets.size();
            mesh.Meshlets.Size = meshletsData.Meshlets.size();

            mesh.PrimitiveIndices.Offset = mPrimitiveIndices.size();
            mesh.PrimitiveIndices.Size = meshletsData.PrimitiveIndices.size();

            mesh.UniqueVertexIndices.Offset = mUniqueVertexIndexBuffer.size();
            mesh.UniqueVertexIndices.Size = meshletsData.UniqueVertexIB.size() * mesh.IndexSize;

            mMeshlets.insert(mMeshlets.end(), meshletsData.Meshlets.begin(), meshletsData.Meshlets.end());
            mPrimitiveIndices.insert(mPrimitiveIndices.end(), meshletsData.PrimitiveIndices.begin(), meshletsData.PrimitiveIndices.end());

            mUniqueVertexIndexBuffer.resize(mUniqueVertexIndexBuffer.size() + mesh.UniqueVertexIndices.Size);

            if (mesh.IndexSize == 2)
            {
                std::vector<uint16_t> indices;
                indices.reserve(meshletsData.UniqueVertexIB.size());
                for (int i = 0; i < meshletsData.UniqueVertexIB.size(); ++i)
                {
                    indices.push_back(meshletsData.UniqueVertexIB[i]);
                }

                memcpy(
                    mUniqueVertexIndexBuffer.data() + mesh.UniqueVertexIndices.Offset,
                    reinterpret_cast<const uint8_t*>(indices.data()),
                    mesh.UniqueVertexIndices.Size);
            }
            else
            {
                memcpy(
                    mUniqueVertexIndexBuffer.data() + mesh.UniqueVertexIndices.Offset,
                    reinterpret_cast<const uint8_t*>(meshletsData.UniqueVertexIB.data()),
                    mesh.UniqueVertexIndices.Size);
            }

            


            if (mesh.IndexSize == 2 && mesh.UniqueVertexIndices.Size % 4 == 1)
            {
                mUniqueVertexIndexBuffer.push_back(0);
                mUniqueVertexIndexBuffer.push_back(0);
            }

            mMeshes.push_back(mesh);
        }
    }
}
