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
        '%{IMGUI_PATH}'
    }

    files {
        '%{IMGUI_PATH}/**'
    }

    filter "configurations:Debug"
        defines "DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "RELEASE"
        runtime "Release"
        optimize "on"