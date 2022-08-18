#include "GraphBuilder.h"

#include <Graph/Resource.h>
#include <Types.h>
#include <Exceptions.h>

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stack>

namespace Engine::Graph
{
    GraphBuilder::GraphBuilder() : mNodes {}, mOrderedIndexes {0}, mQueueCount {0}, mLayersCount {0}
    {

    }

    void GraphBuilder::AddNode(Node&& node)
    {
        mNodes.push_back(node);
    }
    
    void GraphBuilder::SortNodes()
    {
        mOrderedIndexes.clear();
        mQueueCount = 0;
        mLayersCount = 0;

        auto adjacencyLists = AdjacencyList(mNodes.size());

        auto readList = std::unordered_map<Resource, std::vector<Index>>();

        for (Size i = 0; i < mNodes.size(); ++i)
        {
            auto& node = mNodes[i];
            node.GetRelations().OriginalIndex = i;
            for (const auto& readResource : node.GetReadResources())
            {
                if (!readList.contains(readResource))
                {
                    readList.insert({ readResource, std::vector<Index>() });
                }
                readList.at(readResource).push_back(i);
            }

            for (const auto& readResource : node.GetAliasedWriteResources())
            {
                if (!readList.contains(readResource))
                {
                    readList.insert({ readResource, std::vector<Index>() });
                }
                readList.at(readResource).push_back(i);
            }

            mQueueCount = std::max(mQueueCount, node.GetQueueIndex() + 1);
        }

        for (Size i = 0; i < mNodes.size(); ++i)
        {
            auto& node = mNodes[i];

            for (const auto& writeResource : node.GetWriteResources())
            {
                for (Index nodeIndex : readList[writeResource])
                {
                    auto& otherNode = mNodes[nodeIndex];

                    adjacencyLists[i].push_back(nodeIndex);

                    if (node.GetQueueIndex() != otherNode.GetQueueIndex())
                    {
                        node.GetRelations().SyncRequired = true;
                        otherNode.GetRelations().NodesToSyncWith.push_back(&node);
                    }
                }

                adjacencyLists[i].insert(adjacencyLists[i].end(), readList[writeResource].begin(), readList[writeResource].end());
            }
        }

        for (Size i = 0; i < mNodes.size(); ++i)
        {
            auto& node = mNodes[i];
            for (const auto& readResource : node.GetAliasedWriteResources())
            {
                for (Index nodeIndex : readList[readResource])
                {
                    if (node.GetRelations().OriginalIndex == nodeIndex)
                    {
                        continue;
                    }
                    adjacencyLists[nodeIndex].push_back(node.GetRelations().OriginalIndex);

                    auto& otherNode = mNodes[nodeIndex];
                    if (node.GetQueueIndex() != otherNode.GetQueueIndex())
                    {
                        otherNode.GetRelations().SyncRequired = true;
                        node.GetRelations().NodesToSyncWith.push_back(&otherNode);
                    }
                }
            }
        }
        

        auto visited = std::vector<bool>(mNodes.size(), false);

        auto onStack = std::vector<bool>(mNodes.size(), false);

        auto distance = std::vector<int>(mNodes.size(), 0);

        for (Index i = 0; i < mNodes.size(); ++i)
        {
            if (!visited[i])
            {
                DepthFirstSearch(i, visited, onStack, adjacencyLists);
            }
        }

        std::reverse(mOrderedIndexes.begin(), mOrderedIndexes.end());

        for (int i = 0; i < mOrderedIndexes.size(); ++i)
        {
            auto originalIndex = mOrderedIndexes[i];
            auto& node = mNodes[originalIndex];
            node.GetRelations().OrderedIndex = i;

            for (auto& dependentNodeIndex : adjacencyLists[originalIndex])
            {
                auto& dependentNode = mNodes[dependentNodeIndex];

                if (dependentNode.GetRelations().Layer <= node.GetRelations().Layer)
                {
                    dependentNode.GetRelations().Layer = node.GetRelations().Layer + 1;
                }
            }

            mLayersCount = std::max(mLayersCount, node.GetRelations().Layer + 1);
        }
    }

    
    
    void GraphBuilder::Build()
    {
        SortNodes();
        BuildRelationships();
        CullRedundantSyncs();
    }

    void GraphBuilder::Clear()
    {
        mNodes.clear();
        mOrderedIndexes.clear();
        mLayers.clear();
        mQueueCount = 0;
        mLayersCount = 0;
    }

    void GraphBuilder::DepthFirstSearch(Index index, std::vector<bool>& visited, std::vector<bool>& onStack, AdjacencyList& adjacencyList)
    {
        auto& node = mNodes[index];

        onStack[index] = true;
        visited[index] = true;

        if (!adjacencyList[index].empty())
        {
            for (const auto& dependentNodeIndex : adjacencyList[index])
            {
                assert_format(!(visited[dependentNodeIndex] && onStack[dependentNodeIndex]), "Graph circular dependency found");
                if (!visited[dependentNodeIndex])
                {
                    DepthFirstSearch(dependentNodeIndex, visited, onStack, adjacencyList);
                }
            }
        }

        onStack[index] = false;
        mOrderedIndexes.push_back(index);
    }

    void GraphBuilder::BuildRelationships()
    {
        mLayers.resize(mLayersCount);
        std::vector<int32> indexPerLayer(mLayersCount, 0);
        std::vector<Node*> previousNodeInQueue(mQueueCount, nullptr);
        std::vector<int32> indexPerQueue(mQueueCount, 0);
        Index executionIndex = 0;

        for (int i = 0; i < mOrderedIndexes.size(); ++i)
        {
            auto originalIndex = mOrderedIndexes[i];
            auto& node = mNodes[originalIndex];

            node.GetRelations().IndexInLayer = indexPerLayer[node.GetRelations().Layer]++;
            mLayers[node.GetRelations().Layer].Add(&node);
        }

        for (auto& layer : mLayers)
        {
            for (auto* node : layer.GetNodes())
            {
                auto* previousNode = previousNodeInQueue[node->GetQueueIndex()];
                if (previousNode)
                {
                    node->GetRelations().NodesToSyncWith.push_back(previousNode);
                }

                previousNodeInQueue[node->GetQueueIndex()] = node;
                node->GetRelations().IndexInQueue = indexPerQueue[node->GetQueueIndex()]++;

                node->GetRelations().ExecutionIndex = executionIndex++;

                node->GetRelations().DebugNodesToSyncWith = node->GetRelations().NodesToSyncWith;
            }
        }
    }

    void GraphBuilder::CullRedundantSyncs()
    {
        for (auto& layer : mLayers)
        {
            for (auto* node : layer.GetNodes())
            {
                node->GetRelations().SynchronizationIndexSet.resize(mQueueCount, Node::RelationsInGraph::InvalidIndex);
            }

            // First pass
            for (auto* node : layer.GetNodes())
            {
                std::vector<const Node*> closestNodeToSync(mQueueCount, nullptr);

                for (auto* dependentNode : node->GetRelations().NodesToSyncWith)
                {
                    auto* closestNode = closestNodeToSync[dependentNode->GetQueueIndex()];

                    if (!closestNode || dependentNode->GetRelations().IndexInQueue > closestNode->GetRelations().IndexInQueue)
                    {
                        closestNodeToSync[dependentNode->GetQueueIndex()] = dependentNode;
                    }
                }

                node->GetRelations().NodesToSyncWith.clear();

                for (int32 queueIndex = 0; queueIndex < mQueueCount; ++queueIndex)
                {
                    auto* dependentNode = closestNodeToSync[queueIndex];

                    if (dependentNode)
                    {
                        if (dependentNode->GetQueueIndex() != node->GetQueueIndex())
                        {
                            node->GetRelations().SynchronizationIndexSet[queueIndex] = dependentNode->GetRelations().IndexInQueue;
                        }

                        node->GetRelations().NodesToSyncWith.push_back(dependentNode);
                    }
                    else
                    {
                        auto* prevNode = closestNodeToSync[node->GetQueueIndex()];

                        if (prevNode)
                        {
                            auto prevNodeDependentNodeIndexInQueue = prevNode->GetRelations().SynchronizationIndexSet[queueIndex];
                            node->GetRelations().SynchronizationIndexSet[queueIndex] = prevNodeDependentNodeIndexInQueue;
                        }
                    }
                }

                node->GetRelations().SynchronizationIndexSet[node->GetQueueIndex()] = node->GetRelations().IndexInQueue;
            }

            // Second pass
            for (auto* node : layer.GetNodes())
            {
                using SyncCoverage = std::tuple<const Node*, Index, std::vector<int32>>;
                std::unordered_set<int32> queueToSyncWith;

                std::vector<const Node*> nodesToSyncWith;

                std::vector<SyncCoverage> syncCoverages;


                for (const auto* dependentNode : node->GetRelations().NodesToSyncWith)
                {
                    queueToSyncWith.emplace(dependentNode->GetQueueIndex());
                }

                while (!queueToSyncWith.empty())
                {
                    syncCoverages.clear();
                    uint32 maxCoveredSyncs = 0;

                    for (Index nodeIndex = 0; nodeIndex < node->GetRelations().NodesToSyncWith.size(); ++nodeIndex)
                    {
                        const auto* dependentNode = node->GetRelations().NodesToSyncWith[nodeIndex];

                        std::vector<int32> syncedQueues;

                        for (int32 queueIndex : queueToSyncWith)
                        {
                            int currentNodeSyncIndex = node->GetRelations().SynchronizationIndexSet[queueIndex];
                            int dependentNodeSyncIndex = dependentNode->GetRelations().SynchronizationIndexSet[queueIndex];

                            if (queueIndex == node->GetQueueIndex())
                            {
                                currentNodeSyncIndex -= 1;
                            }

                            if (dependentNodeSyncIndex != Node::RelationsInGraph::InvalidIndex && currentNodeSyncIndex <= dependentNodeSyncIndex)
                            {
                                syncedQueues.push_back(queueIndex);
                            }
                        }

                        syncCoverages.push_back(SyncCoverage{ dependentNode, nodeIndex, syncedQueues });
                        maxCoveredSyncs = std::max(maxCoveredSyncs, (uint32)syncedQueues.size());
                    }

                    for (const auto& [dependentNode, dependentNodeIndex, syncedQueues] : syncCoverages)
                    {
                        if (syncedQueues.size() >= maxCoveredSyncs)
                        {
                            if (dependentNode->GetQueueIndex() != node->GetQueueIndex())
                            {
                                nodesToSyncWith.push_back(dependentNode);


                                auto& index = node->GetRelations().SynchronizationIndexSet[dependentNode->GetQueueIndex()];
                                index = std::max(index, dependentNode->GetRelations().SynchronizationIndexSet[dependentNode->GetQueueIndex()]);
                            }

                            for (auto queueIndex : syncedQueues)
                            {
                                queueToSyncWith.erase(queueIndex);
                            }
                        }
                    }

                    for (auto syncCoverageIt = syncCoverages.rbegin(); syncCoverageIt != syncCoverages.rend(); ++syncCoverageIt)
                    {
                        const auto& [dependentNode, dependentNodeIndex, syncedQueues] = *syncCoverageIt;
                        if (syncedQueues.size() >= maxCoveredSyncs)
                        {
                            node->GetRelations().NodesToSyncWith.erase(node->GetRelations().NodesToSyncWith.begin() + dependentNodeIndex);
                        }
                    }
                }

                node->GetRelations().NodesToSyncWith = nodesToSyncWith;

                
            }
        }
    }

    void GraphBuilder::Layer::Add(Node* node)
    {
        mNodes.push_back(node);

        auto it = mNodesPerQueue.find(node->GetQueueIndex());
        if (it == mNodesPerQueue.end())
        {
            mNodesPerQueue.emplace(node->GetQueueIndex(), std::vector<Node*>{});
        }
        mNodesPerQueue.at(node->GetQueueIndex()).push_back(node);
    }
}