#include "Node.h"

namespace Engine::Graph
{
    Node::Node(Name&& name, int queueIndex) : mName{ name }, mQueueIndex{queueIndex}, mRelationsInGraph{ 0 }
    {

    }

    void Node::ReadResource(const Resource& resource)
    {
        mReadResources.push_back(resource.GetName());
    }

    void Node::WriteResource(const Resource& resource)
    {
        mWriteResources.push_back(resource.GetName());
    }
} // namespace Engine::Graph
