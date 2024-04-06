VCGLIB_PATH = "%{LIBRARY_PATH}/vcglib"

project "vcglib"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    targetdir (bin_location .. "/%{prj.name}")
    objdir (obj_location .. "/%{prj.name}")

    includedirs 
    {
        '%{VCGLIB_PATH}/vcg',
        '%{VCGLIB_PATH}/eigenlib',
        
    }

    files {
        '%{VCGLIB_PATH}/vcg/**.h',
        '%{VCGLIB_PATH}/wrap/callback.h',
        '%{VCGLIB_PATH}/eigenlib/**'
    }

    defines {
    }

    links {
        "GKlib"
    }

    filter "configurations:Debug"
        defines "DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "RELEASE"
        runtime "Release"
        optimize "on"