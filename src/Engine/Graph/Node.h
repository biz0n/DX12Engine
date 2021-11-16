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
            void WriteResource(const Resource& resource);
            const Name& GetName() const { return mName; }
            const std::vector<Name>& GetReadResources() const { return mReadResources; }
            const std::vector<Name>& GetWriteResources() const { return mWriteResources; }

            int GetQueueIndex() const { return mQueueIndex; }

            const RelationsInGraph& GetRelations() const { return mRelationsInGraph; }
            RelationsInGraph& GetRelations() { return mRelationsInGraph;  }
            

        private:
            Name mName;
            int mQueueIndex;
            std::vector<Name> mReadResources;
            std::vector<Name> mWriteResources;
            RelationsInGraph mRelationsInGraph;
    };
}