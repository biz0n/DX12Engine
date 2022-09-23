LIBASSERT_PATH = "%{LIBRARY_PATH}/libassert"

project "libassert"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    targetdir (bin_location .. "/%{prj.name}")
    objdir (obj_location .. "/%{prj.name}")

    includedirs 
    {
        '%{LIBASSERT_PATH}/include'
    }

    files {
        '%{LIBASSERT_PATH}/include/*.hpp',
        '%{LIBASSERT_PATH}/src/*.cpp'
    }

    defines {
        -- 'ASSERT_DECOMPOSE_BINARY_LOGICAL', -- Enables expression decomposition of && and ||
        -- 'ASSERT_LOWERCASE', -- Enables assert alias for ASSERT
        'ASSERT_USE_MAGIC_ENUM', -- Use the MagicEnum library to print better diagnostics for enum classes
        --'ASSERT_FAIL=ASSERT_FAIL',
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