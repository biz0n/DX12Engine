#pragma once

#include <Types.h>
#include <Graph/Node.h>

#include <vector>

namespace Engine::Graph
{
    class GraphBuilder
    {
        private:
            struct Layer
            {
            public:
                void Add(Node* node);

                const std::vector<Node*>& GetNodes() const { return mNodes; }
            private:
                std::vector<Node*> mNodes;
                std::unordered_map<int32, std::vector<Node*>> mNodesPerQueue;
            };

        public:
            GraphBuilder();
        
        public:
            void AddNode(Node&& node);

            const std::vector<Node>& GetNodes() const { return mNodes; }
            Node* GetNode(Index i) { return &mNodes[i]; }
            const std::vector<Index>& GetOrderedIndexes() const { return mOrderedIndexes; }

            void Build();
            void Clear();
        
        private:
            using AdjacencyList = std::vector<std::vector<Index>>;
            void DepthFirstSearch(Index index, std::vector<bool>& visited, std::vector<bool>& onStack, AdjacencyList& adjacencyList);
            void SortNodes();
            void BuildRelationships();
            void CullRedundantSyncs();

        private:
            std::vector<Node> mNodes;
            std::vector<Index> mOrderedIndexes;
            std::vector<Layer> mLayers;
            int32 mQueueCount;
            int32 mLayersCount;
    };
}