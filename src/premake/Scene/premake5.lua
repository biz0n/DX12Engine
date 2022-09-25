project "Scene"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    location "%{SCENE_PATH}"
    usestandardpreprocessor "On"

    targetdir (bin_location .. "/%{prj.name}")
    objdir (obj_location .. "/%{prj.name}")

    includedirs 
    {
        '%{SCENE_PATH}'
    }

    files {
        '%{SCENE_PATH}/**.h',
        '%{SCENE_PATH}/**.cpp'
    }

    filter "configurations:Debug"
        defines "DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "RELEASE"
        runtime "Release"
        optimize "on"