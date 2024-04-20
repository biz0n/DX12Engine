project "Common"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    location "%{COMMON_PATH}"
    usestandardpreprocessor "On"

    targetdir (bin_location .. "/%{prj.name}")
    objdir (obj_location .. "/%{prj.name}")

    includedirs 
    {
        '%{COMMON_PATH}'
    }

    files {
        '%{COMMON_PATH}/**.h',
        '%{COMMON_PATH}/**.cpp'
    }

    filter "configurations:Debug"
        defines "DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "RELEASE"
        runtime "Release"
        optimize "on"