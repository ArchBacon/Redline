workspace "redline"
    architecture "x86_64"
    configurations { "Debug", "Release" }
    startproject "redline"
    language "C++"
    cppdialect "C++17"
    cdialect "C17"
    staticruntime "Off"
    characterset "Unicode"
    rtti "On"
    multiprocessorcompile "On"
    warnings "High"
    conformancemode "On"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"
        defines { "_DEBUG", "DEBUG" }

    filter "configurations:Release"
        runtime "Release"
        symbols "On"
        optimize "Full"
        defines { "NDEBUG" }
        buildoptions { "/GL", "/Gy", "/Oi" }
        linkoptions { "/LTCG", "/OPT:REF", "/OPT:ICF" }

    filter {}

project "Jolt"
    kind "StaticLib"
    location "bee"
    targetdir "bee/external/Jolt/lib/x64/%{cfg.buildcfg}"
    objdir "bee/external/Jolt/intermediate/x64/%{cfg.buildcfg}"
    includedirs { "bee/external" }
    pchheader "Jolt/Jolt.h"
    pchsource "bee/external/Jolt/pch.cpp"
    files
    {
        "bee/external/Jolt/**.h",
        "bee/external/Jolt/**.cpp",
        "bee/external/Jolt/**.inl",
    }

project "bee"
    kind "StaticLib"
    location "bee"
    targetdir "bee/lib/x64/%{cfg.buildcfg}"
    objdir "bee/intermediate/x64/%{cfg.buildcfg}"

    includedirs
    {
        "bee/include",
        "bee/external",
        "bee/external/fmt/include",
        "bee/external/csv_parser/include",
        "bee/external/clipper/include",
        "bee/external/Jolt",
        "bee/external/glad/include",
    }

    defines
    {
        "BEE_PROFILE", "BEE_JOLT_PHYSICS", "BEE_PLATFORM_PC",
        "BEE_GRAPHICS_OPENGL", "GLM_FORCE_SILENT_WARNINGS", "_UNICODE", "UNICODE",
    }

    files
    {
        "bee/source/ai/**.cpp",
        "bee/source/core/**.cpp",
        "bee/source/graph/**.cpp",
        "bee/source/math/**.cpp",
        "bee/source/physics/**.cpp",
        "bee/source/rendering/**.cpp",
        "bee/source/tools/**.cpp",
        "bee/source/platform/pc/**.cpp",
        "bee/source/platform/opengl/**.cpp",
        "bee/include/**.hpp",
        "bee/include/**.h",
        "bee/external/clipper/src/**.cpp",
        "bee/external/imgui/*.cpp",
        "bee/external/predicates/predicates.cpp",
        "bee/external/tinygltf/tiny_gltf.cc",
        "bee/external/glad/src/glad.c",
        "bee/external/entt/natvis/entt/**.natvis",
        "bee/external/glm/glm.natvis",
    }

    filter "configurations:Debug"
        defines { "BEE_DEBUG", "BEE_INSPECTOR" }
    filter "configurations:Release"
        defines { "BEE_INSPECTOR" }
    filter {}

project "redline"
    kind "ConsoleApp"
    location "redline"
    targetdir "redline/executable/x64/%{cfg.buildcfg}"
    objdir "redline/intermediate/x64/%{cfg.buildcfg}"
    dependson { "bee", "Jolt" }

    includedirs
    {
        "bee/include",
        "bee/external",
        "bee/external/fmt/include",
        "bee/external/csv_parser/include",
        "bee/external/clipper/include",
        "bee/external/Jolt",
        "bee/external/glad/include",
    }

    defines
    {
        "BEE_PROFILE", "BEE_JOLT_PHYSICS", "BEE_PLATFORM_PC",
        "BEE_GRAPHICS_OPENGL", "GLM_FORCE_SILENT_WARNINGS", "_UNICODE", "UNICODE",
    }

    files
    {
        "redline/**.cpp",
        "redline/**.hpp",
        "redline/**.h",
    }

    libdirs
    {
        "bee/lib/x64/%{cfg.buildcfg}",
        "bee/external/Jolt/lib/x64/%{cfg.buildcfg}",
        "bee/external",
        "bee/external/GLFW",
    }

    links { "bee", "Jolt", "opengl32" }

    postbuildcommands
    {
        '{COPYFILE} "%{wks.location}bee/external/Superluminal/PerformanceAPI.dll" "%{cfg.targetdir}"',
    }

    filter "configurations:Debug"
        defines { "BEE_DEBUG", "BEE_INSPECTOR" }
        links { "glfw3", "fmod/lib/fmodstudioL_vc", "fmod/lib/fmodL_vc", "Superluminal/PerformanceAPI_MDd" }
        ignoredefaultlibraries { "MSVCRT" }
        postbuildcommands
        {
            '{COPYFILE} "%{wks.location}bee/external/fmod/lib/fmodL.dll" "%{cfg.targetdir}"',
            '{COPYFILE} "%{wks.location}bee/external/fmod/lib/fmodstudioL.dll" "%{cfg.targetdir}"',
        }

    filter "configurations:Release"
        defines { "BEE_INSPECTOR" }
        links { "glfw3", "fmod/lib/fmodstudio_vc", "fmod/lib/fmod_vc", "Superluminal/PerformanceAPI_MD" }
        postbuildcommands
        {
            '{COPYFILE} "%{wks.location}bee/external/fmod/lib/fmod.dll" "%{cfg.targetdir}"',
            '{COPYFILE} "%{wks.location}bee/external/fmod/lib/fmodstudio.dll" "%{cfg.targetdir}"',
        }

    filter {}