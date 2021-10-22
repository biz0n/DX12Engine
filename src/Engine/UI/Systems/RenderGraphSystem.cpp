#include "RenderGraphSystem.h"

#include <Scene/SceneObject.h>

#include <imgui/imgui.h>
#include <imgui/imgui_node_editor.h>

namespace ed = ax::NodeEditor;
static ed::EditorContext* g_Context = nullptr;

namespace Engine::UI::Systems
{
    RenderGraphSystem::RenderGraphSystem() : System()
    {
        g_Context = ed::CreateEditor();
    }

    RenderGraphSystem::~RenderGraphSystem()
    {
        ed::DestroyEditor(g_Context);
    }

    void RenderGraphSystem::Process(Scene::SceneObject *scene, const Timer& timer)
    {
        ed::SetCurrentEditor(g_Context);

        ImGui::Begin("Editor");
        ed::Begin("My Editor");
        {
            int uniqueId = 1;

            // Start drawing nodes.
            ed::BeginNode(uniqueId++);
                ImGui::Text("Node A");
                ed::BeginPin(uniqueId++, ed::PinKind::Input);
                    ImGui::Text("-> In");
                ed::EndPin();
                ImGui::SameLine();
                ed::BeginPin(uniqueId++, ed::PinKind::Output);
                    ImGui::Text("Out ->");
                ed::EndPin();
            ed::EndNode();

            ed::BeginNode(uniqueId++);
                ImGui::Text("Node B");
                ed::BeginPin(uniqueId++, ed::PinKind::Input);
                    ImGui::Text("-> In");
                ed::EndPin();
                ImGui::SameLine();
                ed::BeginPin(uniqueId++ , ed::PinKind::Output);
                    ImGui::Text("Out ->");
                ed::EndPin();
            ed::EndNode();

            ed::Link(uniqueId++, 3, 5);

        }
        ed::End();
        ImGui::End();
    }
}