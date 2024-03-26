MESHOPTIMIZER_PATH = "%{LIBRARY_PATH}/meshoptimizer"

project "meshoptimizer"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    targetdir (bin_location .. "/%{prj.name}")
    objdir (obj_location .. "/%{prj.name}")

    includedirs 
    {
        '%{MESHOPTIMIZER_PATH}/src'
    }

    files {
        '%{MESHOPTIMIZER_PATH}/src/*.h',
        '%{MESHOPTIMIZER_PATH}/src/*.cpp'
    }

    defines {

    }

    links {
        'dbghelp'
    }

    filter "configurations:Debug"
        defines "DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "RELEASE"
        runtime "Release"
        optimize "on"