IMGUI_PATH = "%{LIBRARY_PATH}/imgui"

project "ImGui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    warnings 'Off'

    targetdir (bin_location .. "/%{prj.name}")
    objdir (obj_location .. "/%{prj.name}")

    includedirs 
    {
        '%{IMGUI_PATH}/imgui'
    }

    files {
        '%{IMGUI_PATH}/imgui/*.h',
        '%{IMGUI_PATH}/imgui/*.cpp',
        '%{IMGUI_PATH}/imgui/backends/imgui_impl_dx12.h',
        '%{IMGUI_PATH}/imgui/backends/imgui_impl_dx12.cpp',
        '%{IMGUI_PATH}/imgui/backends/imgui_impl_win32.h',
        '%{IMGUI_PATH}/imgui/backends/imgui_impl_win32.cpp',
    }

    filter "configurations:Debug"
        defines "DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "RELEASE"
        runtime "Release"
        optimize "on"


project "ImGuizmo"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    warnings 'Off'

    targetdir (bin_location .. "/%{prj.name}")
    objdir (obj_location .. "/%{prj.name}")

    includedirs 
    {
        '%{IMGUI_PATH}/imgui'
    }

    files {
        '%{IMGUI_PATH}/ImGuizmo/*.h',
        '%{IMGUI_PATH}/ImGuizmo/*.cpp'
    }

    filter "configurations:Debug"
        defines "DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "RELEASE"
        runtime "Release"
        optimize "on"

project "ImGuiNodeEditor"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    warnings 'Off'

    targetdir (bin_location .. "/%{prj.name}")
    objdir (obj_location .. "/%{prj.name}")

    includedirs 
    {
        '%{IMGUI_PATH}/imgui'
    }

    files {
        '%{IMGUI_PATH}/imgui-node-editor/*.h',
        '%{IMGUI_PATH}/imgui-node-editor/*.cpp'
    }

    filter "configurations:Debug"
        defines "DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "RELEASE"
        runtime "Release"
        optimize "on"