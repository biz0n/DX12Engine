
ENGINE_PATH = "%{wks.location}/src/Engine"
SCENE_PATH = "%{wks.location}/src/Scene"
SCENE_CONVERTER_PATH = "%{wks.location}/src/SceneConverter"
LIBRARY_PATH = "%{wks.location}/src/Libraries"
SHADERS_PATH = "%{ENGINE_PATH}/Render/Shaders"
RESOURCES_PATH = "%{wks.location}/Resources/"

IncludeDir = {}
IncludeDir["AssImp"] = "%{LIBRARY_PATH}/assimp/include"
IncludeDir["AssImpEx"] = "%{wks.location}/src/premake/Libraries/AssImp/_config_headers"
IncludeDir["DirectXHeaders"] = "%{LIBRARY_PATH}/DirectX-Headers/include/directx"
IncludeDir["DirectXMesh"] = "%{LIBRARY_PATH}/DirectXMesh/DirectXMesh"
IncludeDir["DirectXShaderCompiler"] = "%{LIBRARY_PATH}/DirectXShaderCompiler/inc"
IncludeDir["DirectXTex"] = "%{LIBRARY_PATH}/DirectXTex/DirectXTex"
IncludeDir["EnTT"] = "%{LIBRARY_PATH}/entt/single_include"
IncludeDir["ImGui"] = "%{LIBRARY_PATH}/imgui"
IncludeDir["PIX"] = "%{LIBRARY_PATH}/PIX"
IncludeDir["libassert"] = "%{LIBRARY_PATH}/libassert/include"
IncludeDir["spdlog"] = "%{LIBRARY_PATH}/spdlog/include"
IncludeDir["freetype"] = "%{LIBRARY_PATH}/freetype/include"
IncludeDir["meshoptimizer"] = "%{LIBRARY_PATH}/meshoptimizer/src"
IncludeDir["METIS"] = "%{LIBRARY_PATH}/METIS/include"
IncludeDir["GKlib"] = "%{LIBRARY_PATH}/GKlib"
IncludeDir["vcglib"] = "%{LIBRARY_PATH}/vcglib"
IncludeDir["eigen"] = "%{LIBRARY_PATH}/vcglib/eigenlib"

StaticLibrary = {}
StaticLibrary["DirectXShaderCompiler"] = "%{LIBRARY_PATH}/DirectXShaderCompiler/lib/x64/dxcompiler.lib"
StaticLibrary["PIX"] = "%{LIBRARY_PATH}/PIX/WinPixEventRuntime.lib"


SharedLibrary = {}
SharedLibrary["dxcompiler"] = "%{LIBRARY_PATH}/DirectXShaderCompiler/bin/x64/dxcompiler.dll"
SharedLibrary["dxil"] = "%{LIBRARY_PATH}/DirectXShaderCompiler/bin/x64/dxil.dll"
SharedLibrary["WinPixEventRuntime"] = "%{LIBRARY_PATH}/PIX/WinPixEventRuntime.dll"
