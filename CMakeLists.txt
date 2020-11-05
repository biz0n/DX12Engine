cmake_minimum_required(VERSION 3.16.0)

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

add_subdirectory( src/Libraries/entt )

set_target_properties( assimp IrrXML uninstall UpdateAssimpLibsDebugSymbolsAndDLLs zlib zlibstatic 
    PROPERTIES
        FOLDER src/Libraries/assimp 
)

add_compile_definitions(DEBUG)

project(d3d12)


set(GUI_TYPE WIN32)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /UMBCS /D_UNICODE /DUNICODE")
set(CMAKE_CXX_STANDARD 20)

file(GLOB IMGUI "src/Libraries/imgui/imgui/*.cpp" "src/Libraries/imgui/imgui/*.h")
file(GLOB PIX "src/Libraries/PIX/*.h")

file(GLOB_RECURSE HEADERS "src/Engine/*.h")
file(GLOB_RECURSE SOURCES "src/Engine/*.cpp")

#aux_source_directory("src/Engine/" SOURCES)
#aux_source_directory("src/Engine/Scene" SOURCES)
#aux_source_directory("src/Engine/Scene/Loader" SOURCES)

add_executable(d3d12 WIN32 ${SOURCES} ${HEADERS} ${IMGUI})

set_property(TARGET d3d12 PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/src/Engine/")

target_include_directories(d3d12 PRIVATE "src/Engine")

target_include_directories(d3d12 PRIVATE "src/Libraries/D3DX12/")

target_include_directories(d3d12 PRIVATE "src/Libraries/imgui/")

target_include_directories(d3d12 PRIVATE "src/Libraries/PIX/")

target_link_libraries(d3d12 PRIVATE "d3d12.lib" "dxgi.lib" "d3dcompiler.lib" "dxguid.lib" "assimp" "DirectXTex" "EnTT" "${CMAKE_SOURCE_DIR}/src/Libraries/PIX/WinPixEventRuntime.lib")

add_custom_command (TARGET d3d12 POST_BUILD
# COPY
COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/src/Libraries/PIX/WinPixEventRuntime.dll	 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug
# Output Message
COMMENT "Copying PIX binaries" VERBATIM)

set_source_files_properties( src/Engine/Resources/Shaders/Forward.hlsl PROPERTIES 
    VS_SHADER_TYPE Pixel
    VS_SHADER_MODEL 5.1
    VS_SHADER_ENTRYPOINT main
)

function(assign_source_group)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel ${ARGV0} "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

assign_source_group("${CMAKE_CURRENT_SOURCE_DIR}/src/Engine" ${SOURCES} ${HEADERS})
assign_source_group("${CMAKE_CURRENT_SOURCE_DIR}/src/Libraries/imgui" ${IMGUI})
assign_source_group("${CMAKE_CURRENT_SOURCE_DIR}/src/Libraries/PIX" ${PIX})
