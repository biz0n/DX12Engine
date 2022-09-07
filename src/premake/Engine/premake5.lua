project "Engine"
    kind "WindowedApp"
    language "C++"
    cppdialect "C++20"
    flags { "NoPCH" }
    systemversion "latest"
    characterset "MBCS"

    targetdir (bin_location .. "/%{prj.name}")
    objdir (obj_location .. "/%{prj.name}")

    files
    {
        "%{ENGINE_PATH}/**.h",
        "%{ENGINE_PATH}/**.cpp",
        "%{ENGINE_PATH}/**.hlsl",
    }

    includedirs
    {
        "%{ENGINE_PATH}",
        "%{IncludeDir.AssImp}",
        "%{IncludeDir.AssImpEx}",
        "%{IncludeDir.DirectXHeaders}",
        "%{IncludeDir.DirectXMesh}",
        "%{IncludeDir.DirectXShaderCompiler}",
        "%{IncludeDir.DirectXTex}",
        "%{IncludeDir.EnTT}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.PIX}",
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
        "DirectXTex", 
        "DirectXMesh",
        "AssImp",
        "ImGui"
    }

    links
    {
        "%{Library.PIX}",
        "%{Library.DirectXShaderCompiler}",
    }

    postbuildcommands { '{COPY} "%{StaticLibrary.dxcompiler}"            "%{cfg.buildtarget.directory}"' }
    postbuildcommands { '{COPY} "%{StaticLibrary.dxil}"                  "%{cfg.buildtarget.directory}"'}
    postbuildcommands { '{COPY} "%{StaticLibrary.WinPixEventRuntime}"    "%{cfg.buildtarget.directory}"' }

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