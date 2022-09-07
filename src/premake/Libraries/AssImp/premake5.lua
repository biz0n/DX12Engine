ASSIMP_PATH = "%{LIBRARY_PATH}/assimp"

project "AssImp"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    warnings 'Off'

    targetdir (bin_location .. "/%{prj.name}")
    objdir (obj_location .. "/%{prj.name}")

    
    includedirs 
    {
        '_config_headers/',
        '_config_headers/assimp/',
        '%{ASSIMP_PATH}',
        '%{ASSIMP_PATH}/include/',
        '%{ASSIMP_PATH}/code',
        '%{ASSIMP_PATH}/contrib/',
        '%{ASSIMP_PATH}/contrib/unzip/',
        '%{ASSIMP_PATH}/contrib/zlib/',
        '%{ASSIMP_PATH}/contrib/rapidjson/include/',
        '%{ASSIMP_PATH}/contrib/pugixml/src/'
    }

    files {
        -- Dependencies
        '%{ASSIMP_PATH}/contrib/unzip/**',
        '%{ASSIMP_PATH}/contrib/zlib/*',
        '%{ASSIMP_PATH}/contrib/rapidjson/**',
        '%{ASSIMP_PATH}/contrib/pugixml/**',
        -- Common
        '%{ASSIMP_PATH}/code/Common/**',
        '%{ASSIMP_PATH}/code/PostProcessing/**',
        '%{ASSIMP_PATH}/code/Material/**',
        '%{ASSIMP_PATH}/code/CApi/**',
        '%{ASSIMP_PATH}/code/Pbrt/**',
        -- Importers
        -- '%{ASSIMP_PATH}/code/AssetLib/**', -- All importers
        '%{ASSIMP_PATH}/code/AssetLib/glTF/**',
        '%{ASSIMP_PATH}/code/AssetLib/glTF2/**',
        '%{ASSIMP_PATH}/code/AssetLib/Assbin/**' -- For caching
    }

    defines {
        "RAPIDJSON_HAS_STDSTRING",
        "ASSIMP_BUILD_NO_OWN_ZLIB"
    }
    -- Importers
    defines {
        'ASSIMP_BUILD_NO_3D_IMPORTER',
        'ASSIMP_BUILD_NO_3DS_IMPORTER',
        'ASSIMP_BUILD_NO_3MF_IMPORTER',
        'ASSIMP_BUILD_NO_AC_IMPORTER',
        'ASSIMP_BUILD_NO_AMF_IMPORTER',
        'ASSIMP_BUILD_NO_ASE_IMPORTER',
        -- 'ASSIMP_BUILD_NO_ASSBIN_IMPORTER'
        'ASSIMP_BUILD_NO_B3D_IMPORTER',
        'ASSIMP_BUILD_NO_BLEND_IMPORTER',
        'ASSIMP_BUILD_NO_BVH_IMPORTER',
        'ASSIMP_BUILD_NO_C4D_IMPORTER',
        'ASSIMP_BUILD_NO_COB_IMPORTER',
        'ASSIMP_BUILD_NO_COLLADA_IMPORTER',
        'ASSIMP_BUILD_NO_CSM_IMPORTER',
        'ASSIMP_BUILD_NO_DXF_IMPORTER',
        'ASSIMP_BUILD_NO_FBX_IMPORTER',
        -- 'ASSIMP_BUILD_NO_GLTF_IMPORTER',
        'ASSIMP_BUILD_NO_HMP_IMPORTER',
        'ASSIMP_BUILD_NO_IFC_IMPORTER',
        'ASSIMP_BUILD_NO_IRR_IMPORTER',
        'ASSIMP_BUILD_NO_IRRMESH_IMPORTER',
        'ASSIMP_BUILD_NO_LWO_IMPORTER',
        'ASSIMP_BUILD_NO_LWS_IMPORTER',
        'ASSIMP_BUILD_NO_M3D_IMPORTER',
        'ASSIMP_BUILD_NO_MD2_IMPORTER',
        'ASSIMP_BUILD_NO_MD3_IMPORTER',
        'ASSIMP_BUILD_NO_MD5_IMPORTER',
        'ASSIMP_BUILD_NO_MDC_IMPORTER',
        'ASSIMP_BUILD_NO_MDL_IMPORTER',
        'ASSIMP_BUILD_NO_MMD_IMPORTER',
        'ASSIMP_BUILD_NO_MS3D_IMPORTER',
        'ASSIMP_BUILD_NO_NDO_IMPORTER',
        'ASSIMP_BUILD_NO_NFF_IMPORTER',
        'ASSIMP_BUILD_NO_IQM_IMPORTER',
        'ASSIMP_BUILD_NO_OBJ_IMPORTER',
        'ASSIMP_BUILD_NO_OFF_IMPORTER',
        'ASSIMP_BUILD_NO_OGRE_IMPORTER',
        'ASSIMP_BUILD_NO_OPENGEX_IMPORTER',
        'ASSIMP_BUILD_NO_PLY_IMPORTER',
        'ASSIMP_BUILD_NO_Q3BSP_IMPORTER',
        'ASSIMP_BUILD_NO_Q3D_IMPORTER',
        'ASSIMP_BUILD_NO_RAW_IMPORTER',
        'ASSIMP_BUILD_NO_SIB_IMPORTER',
        'ASSIMP_BUILD_NO_SMD_IMPORTER',
        'ASSIMP_BUILD_NO_STEP_IMPORTER',
        'ASSIMP_BUILD_NO_STL_IMPORTER',
        'ASSIMP_BUILD_NO_TERRAGEN_IMPORTER',
        'ASSIMP_BUILD_NO_X_IMPORTER',
        'ASSIMP_BUILD_NO_X3D_IMPORTER',
        'ASSIMP_BUILD_NO_XGL_IMPORTER'
    }
    -- Exporters
    defines {
        'ASSIMP_BUILD_NO_COLLADA_EXPORTER',
        'ASSIMP_BUILD_NO_X_EXPORTER',
        'ASSIMP_BUILD_NO_STEP_EXPORTER',
        'ASSIMP_BUILD_NO_OBJ_EXPORTER',
        'ASSIMP_BUILD_NO_STL_EXPORTER',
        'ASSIMP_BUILD_NO_PLY_EXPORTER',
        'ASSIMP_BUILD_NO_3DS_EXPORTER',
        'ASSIMP_BUILD_NO_GLTF_EXPORTER',
        -- 'ASSIMP_BUILD_NO_ASSBIN_EXPORTER',
        'ASSIMP_BUILD_NO_ASSXML_EXPORTER',
        'ASSIMP_BUILD_NO_X3D_EXPORTER',
        'ASSIMP_BUILD_NO_FBX_EXPORTER',
        'ASSIMP_BUILD_NO_M3D_EXPORTER',
        'ASSIMP_BUILD_NO_3MF_EXPORTER',
        'ASSIMP_BUILD_NO_ASSJSON_EXPORTER'
    }

    filter "configurations:Debug"
        defines "DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "RELEASE"
        runtime "Release"
        optimize "on"