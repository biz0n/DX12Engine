GKLIB_PATH = "%{LIBRARY_PATH}/GKlib"

project "GKlib"
    kind "StaticLib"
    language "C"
    cppdialect "C++20"

    targetdir (bin_location .. "/%{prj.name}")
    objdir (obj_location .. "/%{prj.name}")

    includedirs 
    {
        '%{GKLIB_PATH}',
        '%{GKLIB_PATH}/win32'
    }

    files {
        '%{GKLIB_PATH}/*.h',
        '%{GKLIB_PATH}/*.c',
        '%{GKLIB_PATH}/win32/*.h',
        '%{GKLIB_PATH}/win32/*.c'
    }

    defines {
        "USE_GKREGEX",
        "__thread=__declspec(thread)"
    }

    links {
    }

    filter "configurations:Debug"
        defines "DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "RELEASE"
        runtime "Release"
        optimize "on"