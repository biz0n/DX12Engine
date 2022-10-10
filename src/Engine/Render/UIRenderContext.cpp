#include "UIRenderContext.h"

#include <PathResolver.h>
#include <View.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_dx12.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <ImGuizmo/ImGuizmo.h>
#include <UI/Fonts/IconsFontAwesome6.h>


#include <cassert>
#include <d3dx12.h>

namespace Engine::Render
{
    UIRenderContext::UIRenderContext(
        View view, 
        ComPtr<ID3D12Device> device, 
        Memory::DescriptorAllocatorPool* descriptorAllocatorPool,
        uint32 numFramesInFlight, 
        DXGI_FORMAT rtvFormat)
        : mDevice(device), mDescriptorAllocatorPool{descriptorAllocatorPool}
    {
        mFontDescriptorAllocation = descriptorAllocatorPool->AllocateSRDescriptor(1);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
       // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
       // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

        mConfigPath = PathResolver::GetResourcePath("ImGui/imgui.ini").string();
        io.IniFilename = mConfigPath.c_str();

        ImGui_ImplWin32_Init(view.WindowHandle);

        ImGui_ImplDX12_Init(
            device.Get(),
            numFramesInFlight,
            rtvFormat,
            descriptorAllocatorPool->GetCbvSrvUavDescriptorHeap(),
            mFontDescriptorAllocation.GetCPUDescriptor(),
            mFontDescriptorAllocation.GetGPUDescriptor());

        //io.Fonts->AddFontDefault();

        io.Fonts->AddFontFromFileTTF(PathResolver::GetResourcePath("Fonts/Roboto/Roboto-Medium.ttf").string().c_str(), 15.0f);

        // merge in icons from Font Awesome
        static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
        ImFontConfig icons_config; 
        icons_config.MergeMode = true; 
        icons_config.PixelSnapH = true;
        icons_config.GlyphMinAdvanceX = 16.0f; // Use if you want to make the icon monospaced
        icons_config.GlyphOffset = { 0, 2 };
        io.Fonts->AddFontFromFileTTF(PathResolver::GetResourcePath("Fonts/FontAwesome/" FONT_ICON_FILE_NAME_FAS).string().c_str(), 16.0f, &icons_config, icons_ranges);
    }

    UIRenderContext::~UIRenderContext()
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void UIRenderContext::BeginFrame()
    {
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();

        ImGuiIO& io = ImGui::GetIO();
        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());

       // ConfigureDockSpace();

    }

    void UIRenderContext::Draw(ComPtr<ID3D12GraphicsCommandList> commandList)
    {
        ID3D12DescriptorHeap* heap[1] = {mDescriptorAllocatorPool->GetCbvSrvUavDescriptorHeap()};
        commandList->SetDescriptorHeaps(1, heap);
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault(NULL, (void*)commandList.Get());
        }
    }

    void UIRenderContext::Resize(uint32 width, uint32 height)
    {
        ImGui_ImplDX12_InvalidateDeviceObjects();

        ImGui_ImplDX12_CreateDeviceObjects();
    }

    void UIRenderContext::ConfigureDockSpace()
    {
        
        auto* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags host_window_flags = 0;
        host_window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
        host_window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        host_window_flags |= ImGuiWindowFlags_NoBackground;
        host_window_flags |= ImGuiWindowFlags_MenuBar;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpaceViewport", NULL, host_window_flags);
        ImGui::PopStyleVar(3);

        ImGuiID dockspace_id = ImGui::GetID("DockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
        

        if (ImGui::BeginMenuBar())
        {
            /*
            if (ImGui::BeginMenu("Options"))
            {
                ImGui::MenuItem("Padding", NULL);
                ImGui::Separator();

                ImGui::Separator();

                ImGui::EndMenu();
            }
            */
            ImGui::EndMenuBar();
            
        }

        ImGui::End();
    }
} // namespace Engine::Render