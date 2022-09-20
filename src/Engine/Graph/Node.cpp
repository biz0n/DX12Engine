#include "Node.h"

namespace Engine::Graph
{
    Node::Node(Name&& name, int queueIndex) : mName{ name }, mQueueIndex{queueIndex}, mRelationsInGraph{ 0 }
    {

    }

    void Node::ReadResource(const Resource& resource)
    {
        mReadResources.push_back(resource);
    }

    void Node::WriteResource(const Resource& resource, std::optional<Name> originalName)
    {
        mWriteResources.push_back(resource);

        if (originalName)
        {
            mAliasedWriteResources.push_back(Resource{ *originalName, resource.Subresource});
        }
    }
} // namespace Engine::Graph
