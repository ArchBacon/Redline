workspace "redline"
    configurations { "Debug", "Release" }
    platforms { "x64" }
    startproject "hello"
    language "C++"
    cppdialect "C++17"
    cdialect "C17"
    staticruntime "Off"
    characterset "Unicode"
    rtti "On"
    multiprocessorcompile "On"
    warnings "High"
    fatalwarnings { "All" }
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

local B = "bee/"

local function bee_common()
    includedirs {
        B .. "include", B .. "external", B .. "external/fmt/include",
        B .. "external/clipper/include", B .. "external/Jolt", B .. "external/glad/include",
    }
    defines {
        "BEE_PROFILE", "BEE_JOLT_PHYSICS", "BEE_PLATFORM_PC",
        "BEE_GRAPHICS_OPENGL", "GLM_FORCE_SILENT_WARNINGS", "_UNICODE", "UNICODE",
    }
    filter "configurations:Debug"  defines { "BEE_DEBUG", "BEE_INSPECTOR" }
    filter "configurations:Release" defines { "BEE_INSPECTOR" }
    filter {}
end

local function game_project(name)
    kind "ConsoleApp"
    location(name)
    targetdir(name .. "/executable/x64/%{cfg.buildcfg}")
    objdir(name .. "/intermediate/x64/%{cfg.buildcfg}")
    bee_common()
    dependson { "bee", "Jolt" }
    libdirs { B .. "lib/x64/%{cfg.buildcfg}", B .. "external/Jolt/lib/x64/%{cfg.buildcfg}", B .. "external", B .. "external/GLFW" }
    links { "bee", "Jolt", "opengl32" }

    filter "configurations:Debug"
        links { "glfw3", "fmod/lib/fmodstudioL_vc", "fmod/lib/fmodL_vc", "Superluminal/PerformanceAPI_MDd" }
        ignoredefaultlibraries { "MSVCRT" }
    filter "configurations:Release"
        links { "glfw3", "fmod/lib/fmodstudio_vc", "fmod/lib/fmod_vc", "Superluminal/PerformanceAPI_MD" }
    filter {}

    local ext = "%{wks.location}bee/external"
    postbuildcommands { '{COPYFILE} "' .. ext .. '/Superluminal/PerformanceAPI.dll" "%{cfg.targetdir}"' }
    filter "configurations:Debug"
        postbuildcommands {
            '{COPYFILE} "' .. ext .. '/fmod/lib/fmodL.dll" "%{cfg.targetdir}"',
            '{COPYFILE} "' .. ext .. '/fmod/lib/fmodstudioL.dll" "%{cfg.targetdir}"',
        }
    filter "configurations:Release"
        postbuildcommands {
            '{COPYFILE} "' .. ext .. '/fmod/lib/fmod.dll" "%{cfg.targetdir}"',
            '{COPYFILE} "' .. ext .. '/fmod/lib/fmodstudio.dll" "%{cfg.targetdir}"',
        }
    filter {}
end

project "Jolt"
    kind "StaticLib"
    location "bee"
    targetdir(B .. "external/Jolt/lib/x64/%{cfg.buildcfg}")
    objdir(B .. "external/Jolt/intermediate/x64/%{cfg.buildcfg}")
    includedirs { B .. "external" }
    pchheader "Jolt/Jolt.h"
    pchsource(B .. "external/Jolt/pch.cpp")
    files { B .. "external/Jolt/**.h", B .. "external/Jolt/**.cpp", B .. "external/Jolt/**.inl" }

project "bee"
    kind "StaticLib"
    location "bee"
    targetdir(B .. "lib/x64/%{cfg.buildcfg}")
    objdir(B .. "intermediate/x64/%{cfg.buildcfg}")
    bee_common()
    files {
        B .. "source/ai/**.cpp", B .. "source/core/**.cpp", B .. "source/graph/**.cpp",
        B .. "source/math/**.cpp", B .. "source/physics/**.cpp", B .. "source/rendering/**.cpp",
        B .. "source/tools/**.cpp", B .. "source/platform/pc/**.cpp", B .. "source/platform/opengl/**.cpp",
        B .. "include/**.hpp", B .. "include/**.h",
        B .. "external/clipper/src/**.cpp",
        B .. "external/imgui/*.cpp",
        B .. "external/predicates/predicates.cpp",
        B .. "external/tinygltf/tiny_gltf.cc",
        B .. "external/glad/src/glad.c",
        B .. "external/entt/natvis/entt/**.natvis",
        B .. "external/glm/glm.natvis",
    }

project "redline"
    game_project("redline")
    files { "redline/**.cpp", "redline/**.hpp" }
