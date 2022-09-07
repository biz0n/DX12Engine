
ENGINE_PATH = "%{wks.location}/src/Engine"
LIBRARY_PATH = "%{wks.location}/src/Libraries"

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

Library = {}
Library["DirectXShaderCompiler"] = "%{LIBRARY_PATH}/DirectXShaderCompiler/lib/x64/dxcompiler.lib"
Library["PIX"] = "%{LIBRARY_PATH}/PIX/WinPixEventRuntime.lib"


StaticLibrary = {}
StaticLibrary["dxcompiler"] = "%{LIBRARY_PATH}/DirectXShaderCompiler/bin/x64/dxcompiler.dll"
StaticLibrary["dxil"] = "%{LIBRARY_PATH}/DirectXShaderCompiler/bin/x64/dxil.dll"
StaticLibrary["WinPixEventRuntime"] = "%{LIBRARY_PATH}/PIX/WinPixEventRuntime.dll"