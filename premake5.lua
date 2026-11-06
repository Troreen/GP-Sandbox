include "Premake/extensions.lua"
workspace "GraphicsSandbox"
 configurations { "Debug", "Release" }
 platforms { "x64" }
 toolset "v145"
 startproject "Sandbox"
 system "Windows"
 architecture "x86_64"
 filter "configurations:Debug"
  defines { "_DEBUG" }
  symbols "On"
 filter "configurations:Release"
  defines { "NDEBUG", "_RELEASE" }
  optimize "Speed"
  symbols "On"
 filter {}
group "Graphics"
 include "Source/Engine/GraphicsEngine/premake5.lua"
group "Utilities"
 include "Source/Utilities/CommonUtilities/premake5.lua"
 include "Source/Utilities/Logger/premake5.lua"
 include "Source/Utilities/ImGui/premake5.lua"
group "Sandbox"
project "Sandbox"
 kind "WindowedApp"
 language "C++"
 cppdialect "C++20"
 location "Source/Sandbox"
 targetdir "Bin/%{cfg.buildcfg}"
 objdir "Intermediate/Sandbox/%{cfg.buildcfg}"
 files { "Source/Sandbox/**.h", "Source/Sandbox/**.cpp" }
 includedirs { "Source/Engine", "Source/Utilities", "Source/Utilities/CommonUtilities", "Source/Utilities/ImGui" }
 libdirs { "Dependencies/TGAFBXImporter/lib/%{cfg.buildcfg}" }
 links { "GraphicsEngine", "Logger", "ImGui", "CommonUtilities", "TGAFbx", "d3d11", "dxgi", "dxguid", "d3dcompiler" }
 defines { "WIN32_LEAN_AND_MEAN", "NOMINMAX" }
 systemversion "latest"
 linkoptions { "/IGNORE:4099" }
 debugdir "Bin/%{cfg.buildcfg}"
 postbuildcommands {
  '{COPYFILE} "%{wks.location}/Dependencies/.dlls/libfbxsdk.dll" "%{cfg.targetdir}"',
  '{COPYDIR} "%{wks.location}/Source/Engine/GraphicsEngine/Shaders" "%{cfg.targetdir}/EngineShaders"',
  '{COPYDIR} "%{wks.location}/Content/Shaders" "%{cfg.targetdir}/EngineShaders"'
 }
