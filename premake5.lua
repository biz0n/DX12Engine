include "dependencies.lua"

workspace "Engine"
    architecture "x64"
    startproject "Engine"

    configurations
    {
        "Debug",
        "Release"
    }

    flags
    {
        "MultiProcessorCompile"
    }

outputdir = "%{cfg.buildcfg}"

bin_location = ("%{wks.location}/bin/" .. outputdir)
obj_location = ("%{wks.location}/bin-int/" .. outputdir)

group "Dependencies"
    include "src/premake/Libraries"

group ""
    include "src/premake/Engine"


project "Premake"
    kind "Utility"

    targetdir (bin_location .. "/%{prj.name}")
    objdir (obj_location .. "/%{prj.name}")

    files
    {
        "%{wks.location}/**.lua"
    }

    postbuildmessage "Regenerating project files with Premake5!"
    postbuildcommands
    {
        '"%{wks.location}premake/premake5.exe" %{_ACTION} --file="%{wks.location}premake5.lua"'
    }