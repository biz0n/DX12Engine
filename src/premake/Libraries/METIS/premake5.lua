METIS_PATH = "%{LIBRARY_PATH}/METIS"

project "METIS"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    targetdir (bin_location .. "/%{prj.name}")
    objdir (obj_location .. "/%{prj.name}")

    includedirs 
    {
        '%{METIS_PATH}/include',
        "%{IncludeDir.GKlib}",
    }

    files {
        '%{METIS_PATH}/libmetis/*.h',
        '%{METIS_PATH}/libmetis/*.c',
        '%{METIS_PATH}/include/*.h',
    }

    defines {
        "IDXTYPEWIDTH=64",
        "REALTYPEWIDTH=64",
        "USE_GKREGEX",
        "__thread=__declspec(thread)"
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