cmake_minimum_required(VERSION 3.15.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_compile_options(/MP)

# Make sure DLL and EXE targets go to the same directory.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # Output directory for static lib (.LIB)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for shared lib (.DLL)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for executables (.EXE)

set( ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "Build Assimp Tools" FORCE )
set( ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "Build Assimp Samples" FORCE )
set( ASSIMP_BUILD_TESTS OFF CACHE BOOL "Build Assimp Tests" FORCE )

add_subdirectory( src/Libraries/assimp )

add_subdirectory( src/Libraries/DirectXTex )

set_target_properties( assimp IrrXML uninstall UpdateAssimpLibsDebugSymbolsAndDLLs zlib zlibstatic 
    PROPERTIES
        FOLDER src/Libraries/assimp 
)

add_compile_definitions(DEBUG)

project(d3d12)

set(GUI_TYPE WIN32)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /UMBCS /D_UNICODE /DUNICODE")
set(CMAKE_CXX_STANDARD 17)

file(GLOB HEADERS "src/*.h")
file(GLOB IMGUI "src/Libraries/imgui/*.h" "src/Libraries/imgui/*.cpp")
aux_source_directory("src" SOURCES)

add_executable(d3d12 WIN32 ${SOURCES} ${HEADERS} ${IMGUI})

set_property(TARGET d3d12 PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/src")

target_link_libraries(d3d12 PRIVATE "d3d12.lib" "dxgi.lib" "d3dcompiler.lib" "assimp" "DirectXTex")

set_source_files_properties( src/Resources/Shaders/VertexShader.hlsl PROPERTIES 
    VS_SHADER_TYPE Vertex
    VS_SHADER_MODEL 5.1
    VS_SHADER_ENTRYPOINT main
)

set_source_files_properties( src/Resources/Shaders/PixelShader.hlsl PROPERTIES 
    VS_SHADER_TYPE Pixel
    VS_SHADER_MODEL 5.1
    VS_SHADER_ENTRYPOINT main
)