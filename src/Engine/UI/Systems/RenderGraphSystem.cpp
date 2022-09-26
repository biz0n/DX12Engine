#include "RenderGraphSystem.h"

#include <Hash.h>
#include <Scene/SceneRegistry.h>

#include <Render/Renderer.h>

#include <Graph/GraphBuilder.h>
#include <Graph/Node.h>
#include <Graph/Resource.h>

#include <UI/RenderGraphUI/widgets.h>

#include <wyhash.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include <imgui/imgui.h>


#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <format>

namespace ed = ax::NodeEditor;

//static const Engine::Graph::GraphBuilder* g_GraphBuilder = nullptr;
//static std::vector<Engine::Graph::Resource>* g_Resources = nullptr;

namespace Engine::UI::Systems
{
    RenderGraphSystem::RenderGraphSystem(SharedPtr<Engine::Render::Renderer> renderer) : System(), mRenderer{ renderer }
    {
        mEditorContext = ed::CreateEditor();

        /*
        g_GraphBuilder = new Graph::GraphBuilder();
        g_Resources = new std::vector<Graph::Resource>();

        auto resource1 = g_Resources->emplace_back(Graph::Resource{ "Resource1", 0 });
        auto resource2 = g_Resources->emplace_back(Graph::Resource{ "Resource2", 0 });
        auto resource3 = g_Resources->emplace_back(Graph::Resource{ "Resource3", 0 });
        auto resource4 = g_Resources->emplace_back(Graph::Resource{ "Resource4", 0 });
        auto resource5 = g_Resources->emplace_back(Graph::Resource{ "Resource5", 0 });
        auto resource6 = g_Resources->emplace_back(Graph::Resource{ "_Resource6", 0 });

        auto node0 = Graph::Node{ "Node0", 0 };
        auto node1 = Graph::Node{ "Node1", 0 };
        auto node2 = Graph::Node{ "Node2", 1 };
        auto node3 = Graph::Node{ "Node3", 1 };
        auto node4 = Graph::Node{ "Node4", 2 };
        auto node5 = Graph::Node{ "Node5", 2 };
        auto node6 = Graph::Node{ "Node6", 3 };
        auto node7 = Graph::Node{ "Node7", 3 };
        auto node8 = Graph::Node{ "Node8", 3 };

        node1.WriteResource(resource1);
        node2.WriteResource(resource2);
        node3.WriteResource(resource3);
        node4.WriteResource(resource4);
        node5.WriteResource(resource5);
        
        node3.ReadResource(resource1);
        node3.ReadResource(resource2);

        node5.ReadResource(resource4);

        node6.ReadResource(resource1);
        node6.ReadResource(resource3);
        node6.ReadResource(resource5);

        node7.WriteResource(resource6, resource4.Id);
        node8.ReadResource(resource6);

        
        g_GraphBuilder->AddNode(std::move(node0));
        g_GraphBuilder->AddNode(std::move(node1));
        g_GraphBuilder->AddNode(std::move(node2));
        g_GraphBuilder->AddNode(std::move(node3));
        g_GraphBuilder->AddNode(std::move(node4));
        g_GraphBuilder->AddNode(std::move(node5));
        g_GraphBuilder->AddNode(std::move(node6));
        g_GraphBuilder->AddNode(std::move(node7));
        g_GraphBuilder->AddNode(std::move(node8));
        

        g_GraphBuilder->Build();
        

        for(auto& node : g_GraphBuilder->GetNodes())
        {
            for (auto& res : node.GetReadResources())
            {
                if (!g_Reads->contains(res))
                {
                    g_Reads->insert({res, std::unordered_set<Engine::Name>()});
                }
                g_Reads->at(res).emplace(node.GetName());
            }
        }
        */

        mShowResourcesLinks = true;
        mShowNodesLinks = true;
        mShowNodesDebugLinks = false;
    }

    RenderGraphSystem::~RenderGraphSystem()
    {
        ed::DestroyEditor(mEditorContext);
    }

    RenderGraphSystem::NodeSizes RenderGraphSystem::CalculateNodeSizes(const Engine::Graph::Node* node)
    {
        NodeSizes sizes;

        float titleWidth = ImGui::CalcTextSize(node->GetName().string().c_str()).x;

        float maxReadResourceName = 0;
        float maxWriteResourceName = 0;

        for (Size index = 0; index < std::max(node->GetReadResources().size(), node->GetWriteResources().size()); ++index)
        {
            if (index < node->GetReadResources().size())
            {
                auto& resource = node->GetReadResources()[index];
                auto resourceName = std::format("{}:{}", resource.Id.c_str(), resource.Subresource);

                float width = ImGui::CalcTextSize(resourceName.c_str()).x;
                maxReadResourceName = std::max(maxReadResourceName, width);
            }

            if (index < node->GetWriteResources().size())
            {
                auto& resource = node->GetWriteResources()[index];
                auto resourceName = std::format("{}:{}", resource.Id.c_str(), resource.Subresource);

                float width = ImGui::CalcTextSize(resourceName.c_str()).x;
                maxWriteResourceName = std::max(maxWriteResourceName, width);
            }
        }

        sizes.TitleWidth = titleWidth;
        sizes.NodeWidth = std::max(titleWidth, maxReadResourceName + 0 + maxWriteResourceName);
        sizes.ReadWidth = maxReadResourceName;
        sizes.WriteWidth = maxWriteResourceName;

        return sizes;
    }

    void RenderGraphSystem::Process(Scene::SceneRegistry *scene, const Timer& timer)
    {

        auto graphBuilder = &mRenderer->GetGraph();
        mReads.clear();
        for (auto& node : graphBuilder->GetNodes())
        {
            for (auto& res : node.GetReadResources())
            {
                if (!mReads.contains(res))
                {
                    mReads.insert({ res, std::unordered_set<Engine::Name>() });
                }
                mReads.at(res).emplace(node.GetName());
            }
        }


        ed::SetCurrentEditor(mEditorContext);

        ImVec2 cursor_pos = ImGui::GetIO().MousePos;


        if (ImGui::Begin("Editor"))
        {
            if (ImGui::BeginChild("Editor settings", {200, 500}))
            {
                bool v = false;
                ImGui::Checkbox("Show resources links", &mShowResourcesLinks);
                ImGui::Checkbox("Show node links", &mShowNodesLinks);
                ImGui::Checkbox("Show node debug links", &mShowNodesDebugLinks);
            }
            ImGui::EndChild();
            ImGui::SameLine();
            
            if (ImGui::BeginChild("Editor child"))
            {
                ed::Begin("My Editor");
                {
                    float spacing = ImGui::GetStyle().ItemSpacing.x;

                    int index = 0;
                    for (auto& node : graphBuilder->GetNodes())
                    {
                        auto sizes = CalculateNodeSizes(&node);

                        ImVec2 headerMin = ImVec2();
                        ImVec2 headerMax = ImVec2();

                        ed::BeginNode(node.GetName().id());
                        {
                            const ImVec2 iconSize = { 15, 15 };
                            index++;

                            // Header
                            ImGui::BeginGroup();
                            {
                                auto syncInId = std::hash_combine(node.GetName(), 0);

                                ed::BeginPin(syncInId, ed::PinKind::Input);
                                {
                                    ax::Widgets::Icon(iconSize, ax::Drawing::IconType::Diamond, true, { 1,0,0,1 }, { 1,0,0,0 });
                                }
                                ed::EndPin();

                                ImGui::SameLine();

                                
                                ImGui::Text(node.GetName().string().c_str());
                                ImGui::SameLine();
                                ImGui::Dummy({ sizes.NodeWidth - sizes.TitleWidth + spacing * 2, 0 });

                                
                                ImGui::SameLine();
                                auto syncOutId = std::hash_combine(node.GetName(), 1);
                                ed::BeginPin(syncOutId, ed::PinKind::Output);
                                {
                                    ax::Widgets::Icon(iconSize, ax::Drawing::IconType::Diamond, true, { 0,1,0,1 }, { 0,1,0,0 });
                                }
                                ed::EndPin();
                            }
                            ImGui::EndGroup();

                            headerMin = ImGui::GetItemRectMin();
                            headerMax = ImGui::GetItemRectMax();

                            // Content
                            ImGui::BeginGroup();
                            {
                                ImGui::Text("Queue: %i", node.GetQueueIndex());
                                ImGui::Text("Layer: %i", node.GetRelations().Layer);
                                ImGui::Text("Execution index: %i", node.GetRelations().ExecutionIndex);
                                ImGui::Text("Queue index: %i", node.GetRelations().IndexInQueue);
                                ImGui::Text("Layer index: %i", node.GetRelations().IndexInLayer);
                                ImGui::Text("Ordered index: %i", node.GetRelations().OrderedIndex);
                            }
                            ImGui::EndGroup();

                            ImGui::Spacing();
                            ImGui::Spacing();
                            ImGui::Spacing();

                            // Pins
                            for (Size index = 0; index < std::max(node.GetReadResources().size(), node.GetWriteResources().size()); ++index)
                            {
                                    
                                if (index < node.GetReadResources().size())
                                {
                                    auto& resource = node.GetReadResources()[index];

                                    auto id = std::hash_combine(node.GetName(), resource);
                                    ed::BeginPin(id, ed::PinKind::Input);
                                    {
                                        ax::Widgets::Icon(iconSize, ax::Drawing::IconType::Flow, false, { 1,0,0,1 }, { 1,0,0,0 });
                                    }
                                    ed::EndPin();

                                    ImGui::SameLine();

                                    ImGui::Text("%s:%i", resource.Id.c_str(), resource.Subresource);
                                    float textWidth = ImGui::GetItemRectSize().x;

                                    if (ImGui::IsItemHovered())
                                    {
                                        ImVec2 tooltip_pos = cursor_pos + ImVec2(16 * ImGui::GetCurrentContext()->Style.MouseCursorScale, 8 * ImGui::GetCurrentContext()->Style.MouseCursorScale);
                                        ImGui::SetNextWindowPos(tooltip_pos);

                                        ImGui::BeginTooltip();
                                        ImGui::Text("Resource Id: %i", resource.Id.id());
                                        ImGui::EndTooltip();
                                    }

                                    ImGui::SameLine();
                                    ImGui::Dummy({ sizes.ReadWidth - textWidth - spacing , 0 });
                                }
                                else
                                {
                                    ImGui::Dummy({ sizes.ReadWidth + spacing + iconSize.x , 0 });
                                }

                                ImGui::SameLine();
                                ImGui::Dummy({ 0, 0 });
                                ImGui::SameLine();

                                if (index < node.GetWriteResources().size())
                                {
                                    auto& resource = node.GetWriteResources()[index];

                                    auto id = std::hash_combine(node.GetName(), resource);

                                    auto resourceName = std::format("{}:{}", resource.Id.c_str(), resource.Subresource);

                                    float textWidth = ImGui::CalcTextSize(resourceName.c_str()).x;
                                    ImGui::Dummy({ sizes.WriteWidth - textWidth - spacing , 0});

                                    ImGui::SameLine();

                                    ImGui::Text(resourceName.c_str());
                                        
                                    
                                    if (ImGui::IsItemHovered())
                                    {
                                        ImVec2 tooltip_pos = cursor_pos + ImVec2(16 * ImGui::GetCurrentContext()->Style.MouseCursorScale, 8 * ImGui::GetCurrentContext()->Style.MouseCursorScale);
                                        ImGui::SetNextWindowPos(tooltip_pos);

                                        ImGui::BeginTooltip();
                                        ImGui::Text("Resource Id: %i %g %g", resource.Id.id(), cursor_pos.x, cursor_pos.y);
                                        ImGui::EndTooltip();
                                    }

                                    ImGui::SameLine();

                                    ed::BeginPin(id, ed::PinKind::Output);
                                    {
                                        ax::Widgets::Icon(iconSize, ax::Drawing::IconType::Flow, false, { 0,1,0,1 }, { 0,1,0,0 });
                                    }
                                    ed::EndPin();
                                }
                                else
                                {
                                    ImGui::Dummy({ sizes.WriteWidth + spacing + iconSize.x , 0});
                                }
                            }
                        }
                        ed::EndNode();

                        if (ImGui::IsItemVisible())
                        {
                            wyhash32_seed(node.GetQueueIndex());
                            float r = wyhash32_float(0.03f, 0.35f);
                            float g = wyhash32_float(0.03f, 0.35f);
                            float b = wyhash32_float(0.03f, 0.35f);


                            auto alpha = static_cast<int>(255 * ImGui::GetStyle().Alpha);

                            auto drawList = ed::GetNodeBackgroundDrawList(node.GetName().id());

                            const auto halfBorderWidth = ed::GetStyle().NodeBorderWidth * 0.5f;

                            auto headerColor = IM_COL32(0, 0, 0, alpha) | (ImColor{ r, g, b, 1.0f } &IM_COL32(255, 255, 255, 0));
                            if ((headerMax.x > headerMin.x) && (headerMax.y > headerMin.y))
                            {
                                drawList->AddRectFilled(headerMin - ImVec2(8 - halfBorderWidth, 8 - halfBorderWidth),
                                    headerMax + ImVec2(8 - halfBorderWidth, 4), headerColor, ed::GetStyle().NodeRounding, ImDrawFlags_RoundCornersTop);


                                auto headerSeparatorMin = ImVec2(headerMin.x, headerMax.y);
                                auto headerSeparatorMax = ImVec2(headerMax.x, headerMin.y);

                            }
                        }
                    }

                    for (auto& node : graphBuilder->GetNodes())
                    {
                        if (mShowResourcesLinks)
                        {
                            for (auto& write : node.GetWriteResources())
                            {
                                if (mReads.contains(write))
                                {
                                    for (auto& read : mReads.at(write))
                                    {
                                        auto linkId = std::hash_combine(write, node.GetName(), read);
                                        ed::Link(linkId, std::hash_combine(node.GetName(), write), std::hash_combine(read, write));
                                    }
                                }
                            }
                        }

                        if (mShowNodesDebugLinks)
                        {
                            for (auto* otherNode : node.GetRelations().DebugNodesToSyncWith)
                            {
                                auto linkId = std::hash_combine(0, node.GetName(), otherNode->GetName(), 1);
                                auto syncInId = std::hash_combine(node.GetName(), 0);
                                auto syncOutId = std::hash_combine(otherNode->GetName(), 1);
                                ed::Link(linkId, syncOutId, syncInId, ImVec4{ 0.5, 0.5, 0.5 ,1 }, 0.5);
                            }
                        }

                        if (mShowNodesLinks)
                        {
                            for (auto* otherNode : node.GetRelations().NodesToSyncWith)
                            {
                                auto linkId = std::hash_combine(0, node.GetName(), otherNode->GetName(), 1);
                                auto syncInId = std::hash_combine(node.GetName(), 0);
                                auto syncOutId = std::hash_combine(otherNode->GetName(), 1);
                                ed::Link(linkId, syncOutId, syncInId, ImVec4{ 0.9, 0.4, 0.4 ,1 }, 0.5);
                            }
                        }
                    }

                }
                ed::End();
            }
            ImGui::EndChild();
        }
        ImGui::End();

        ed::SetCurrentEditor(nullptr);
    }
}