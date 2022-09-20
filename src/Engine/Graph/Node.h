#pragma once

#include <Name.h>
#include <Graph/Resource.h>

#include <vector>
#include <unordered_map>

namespace Engine::Graph
{
    class Node
    {
    public:
        struct RelationsInGraph
        {
            static const int InvalidIndex = std::numeric_limits<int>::max();
            int Layer;
            int OriginalIndex;
            int OrderedIndex;
            int ExecutionIndex;
            int IndexInQueue;
            int IndexInLayer;
            std::vector<const Node*> NodesToSyncWith;
            std::vector<const Node*> DebugNodesToSyncWith;
            std::vector<int> SynchronizationIndexSet;
            bool SyncRequired;
        };

        public:
            Node(Name&& name, int queueIndex);
        
        public:
            void ReadResource(const Resource& resource);
            void WriteResource(const Resource& resource, std::optional<Name> originalName = std::nullopt);
            const Name& GetName() const { return mName; }
            const std::vector<Resource>& GetReadResources() const { return mReadResources; }
            const std::vector<Resource>& GetWriteResources() const { return mWriteResources; }
            const std::vector<Resource>& GetAliasedWriteResources() const { return mAliasedWriteResources; }

            int GetQueueIndex() const { return mQueueIndex; }

            const RelationsInGraph& GetRelations() const { return mRelationsInGraph; }
            RelationsInGraph& GetRelations() { return mRelationsInGraph;  }
            

        private:
            Name mName;
            int mQueueIndex;
            std::vector<Resource> mReadResources;
            std::vector<Resource> mWriteResources;
            std::vector<Resource> mAliasedWriteResources;
            RelationsInGraph mRelationsInGraph;
    };
}