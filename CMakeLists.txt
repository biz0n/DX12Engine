cmake_minimum_required(VERSION 3.15.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_compile_options(/MP)

# Make sure DLL and EXE targets go to the same directory.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # Output directory for static lib (.LIB)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for shared lib (.DLL)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for executables (.EXE)

add_compile_definitions(DEBUG)

project(d3d12)

set(GUI_TYPE WIN32)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /UMBCS /D_UNICODE /DUNICODE")
set(CMAKE_CXX_STANDARD 17)

file(GLOB HEADERS "src/*.h")
file(GLOB IMGUI "src/Libraries/imgui/*.h" "src/Libraries/imgui/*.cpp")
aux_source_directory("src" SOURCES)

add_executable(d3d12 WIN32 ${SOURCES} ${HEADERS} ${IMGUI})

target_link_libraries(d3d12 PRIVATE "d3d12.lib" "dxgi.lib" "d3dcompiler.lib")