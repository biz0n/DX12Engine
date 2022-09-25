project "SceneConverter"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    flags { "NoPCH" }
    systemversion "latest"
    characterset "MBCS"
    location "%{SCENE_CONVERTER_PATH}"
    usestandardpreprocessor "On"

    targetdir (bin_location .. "/%{prj.name}")
    objdir (obj_location .. "/%{prj.name}")

    includedirs 
    {
        '%{SCENE_CONVERTER_PATH}',
        "%{SCENE_PATH}",
        "%{IncludeDir.AssImp}",
        "%{IncludeDir.AssImpEx}",
        "%{IncludeDir.DirectXMesh}",
        "%{IncludeDir.DirectXTex}",
        "%{IncludeDir.libassert}",
        "%{IncludeDir.spdlog}",
    }

    links
    {
        "Scene",
        "DirectXMesh",
        "DirectXTex",
        "AssImp",
        "libassert"
    }

    links
    {
        "d3d11.lib",
        "dxgi.lib", 
    }

    files {
        '%{SCENE_CONVERTER_PATH}/**.h',
        '%{SCENE_CONVERTER_PATH}/**.cpp'
    }

    filter "configurations:Debug"
        defines "DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "RELEASE"
        runtime "Release"
        optimize "on"