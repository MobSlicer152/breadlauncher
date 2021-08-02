include "scripts/vs2022"

-- Template for output folders
outdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}-%{prj.name}"

-- Postfix for libraries
libpostfix = ""
if os.target() == "windows" then
	libpostfix = "-win32"
elseif os.target() == "macosx" then
	libpostfix = "-macos"
elseif os.target() == "linux" then
	libpostfix = "-linux"
else
	libpostfix = "-" .. os.target()
end

-- Setup workspace
workspace "breadlauncher"
	location "project"

	configurations {
		"debug",
		"release",
		"dist"
	}

	platforms {
		"x86",
		"x86_64",
		"arm"
	}

	-- CPU architecture
	filter "platforms:x86"
		defines "BREAD_X86"
		architecture "x86"

	filter "platforms:x86_64"
		defines "BREAD_X86"
		architecture "x86_64"

	filter "platforms:arm"
		defines "BREAD_ARM"
		architecture "ARM64"

	-- Configurations
	
	-- Debug, has full symbols
	filter "configurations:debug"
		defines "BREAD_CONFIG_DEBUG"
		symbols "On"

	-- Release, has some optimization but is still debuggable
	filter "configurations:release"
		defines "BREAD_CONFIG_RELEASE"
		optimize "Debug"

	-- Dist, full optimization
	filter "configurations:dist"
		defines "BREAD_CONFIG_DIST"
		optimize "Full"

	-- OS-specific stuff
	
	-- Windows
	filter "system:windows"
		defines "BREAD_WIN32"
		staticruntime "Off"
		systemversion "latest"

	-- macOS
	filter "system:macosx"
		defines "BREAD_MACOS"
	
	-- Linux
	filter "system:linux"
		defines "BREAD_LINUX"

	-- Regenerate projects after a successful build
	filter {}
		postbuildcommands {
			("cd ../.. && " ..
			 _PREMAKE_COMMAND ..
			 " --os=" ..
			 os.target() ..
			 " " ..
			 _ACTION ..
			 " && cd %{wks.location}/%{prj.name}")
		}

-- Main launcher project
project "launcher"
	-- Basic information about the project
	targetname "bread_launcher"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	location "%{wks.location}/%{prj.name}"

	-- Output directories
	targetdir ("bin/" .. outdir)
	objdir ("bin-int/" .. outdir)

	-- Include our sources
	files {
		"include/**.h",
		"src/**.cc",

		-- Include this script for easy access from an IDE, doesn't affect build
		_MAIN_SCRIPT
	}

	-- Add our include folder
	includedirs {
		"include"
	}

	-- Add the dependency include folder on non-Linux systems (Linux is sane and has /usr/include, wereas other systems don't like consistency enough to be nice like that)
	filter "system:not linux"
		includedirs {
			"deps/include"
		}

	-- Different levels for fat binaries and config-independant
	libdirs {
		"deps/libs",
		"deps/libs/%{cfg.architecture}",
		"deps/libs/%{cfg.architecture}/%{cfg.buildcfg}"
	}

	-- Clean up Finder's metadata, it causes issues with wildcards
	filter "system:macosx"
		prebuildcommands {
			"../../scripts/kill_finder_meta.sh"
		}

	-- Link with stuff
	links {
		("curl" .. libpostfix),
		("fmt" .. libpostfix)
	}

	-- Copy PDBs on Windows
	filter { "system:windows", "configurations:debug or release" }
		prebuildcommands {
			"{COPY} ../../deps/symbols/%{cfg.architecture}/* %{cfg.buildtarget.directory}"
		}	
