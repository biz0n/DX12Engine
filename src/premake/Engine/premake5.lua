project "Engine"
    kind "WindowedApp"
    language "C++"
    cppdialect "C++20"
    flags { "NoPCH" }
    systemversion "latest"
    characterset "MBCS"
    location "%{ENGINE_PATH}"
    usestandardpreprocessor "On"

    targetdir (bin_location .. "/%{prj.name}")
    objdir (obj_location .. "/%{prj.name}")

    files
    {
        "%{ENGINE_PATH}/**.h",
        "%{ENGINE_PATH}/**.cpp",
        "%{ENGINE_PATH}/**.hlsl"
    }

    includedirs
    {
        "%{ENGINE_PATH}",
        "%{SCENE_PATH}",
        "%{IncludeDir.DirectXHeaders}",
        "%{IncludeDir.DirectXShaderCompiler}",
        "%{IncludeDir.DirectXTex}",
        "%{IncludeDir.EnTT}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.ImGui}/imgui",
        "%{IncludeDir.PIX}",
        "%{IncludeDir.libassert}",
        "%{IncludeDir.spdlog}",
    }

    links
    {
        "d3d12.lib",
        "dxgi.lib", 
        "d3dcompiler.lib", 
        "dxguid.lib"
    }

    links
    {
        "Scene",
        "DirectXTex",
        "ImGui",
        "ImGuizmo",
        "ImGuiNodeEditor",
        "libassert",
    }

    links
    {
        "%{Library.PIX}",
        "%{Library.DirectXShaderCompiler}",
    }

    postbuildcommands { '{COPYFILE} "%{StaticLibrary.dxcompiler}"            "%{cfg.buildtarget.directory}"' }
    postbuildcommands { '{COPYFILE} "%{StaticLibrary.dxil}"                  "%{cfg.buildtarget.directory}"'}
    postbuildcommands { '{COPYFILE} "%{StaticLibrary.WinPixEventRuntime}"    "%{cfg.buildtarget.directory}"' }

    postbuildcommands { '{COPYDIR} "%{SHADERS_PATH}"                         "%{cfg.buildtarget.directory}/Shaders"' }

    filter "configurations:Debug"
        defines "DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "RELEASE"
        runtime "Release"
        optimize "on"

    filter "files:**.hlsl" 
        buildaction "None"