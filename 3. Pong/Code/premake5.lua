-- ---------------------------------
-- Marc García Carda
-- TCP - BSc Computer Science For Games
-- SHU 2021-2022
-- Project Configuration.
-- ---------------------------------

solution ("Technical Computing Project - Pong" .. _ACTION)
  configurations { "Debug", "Release" }
  platforms { "x32", "x64" }
  location ("build")
  language "C++"
  kind "ConsoleApp"

	projects = { "Pong_Client", "Pong_Server" }

	for i, prj in ipairs(projects) do
		project (prj)
		targetname (prj)

		prj_path = "./build/" .. prj

		location (prj_path .. "/" .. _ACTION)

		includedirs {
			"./include/",
			"./deps/SFML-2.5.1/include",
			"./deps/imgui",
			"./deps/NetworkLibrary"
		}

		files {
			"./deps/SFML-2.5.1/**.hpp",
			"./deps/imgui/**.h",
			"./deps/imgui/**.cpp",
			"./deps/NetworkLibrary/**.h"
		}

	    defines { "_CRT_SECURE_NO_WARNINGS", "SFML_STATIC" }
	    warnings "Extra"


    configuration "Debug"
        defines { "DEBUG" }
		links { "./deps/SFML-2.5.1/lib/sfml-graphics-s-d", 
		"./deps/SFML-2.5.1/lib/sfml-window-s-d", 
		"./deps/SFML-2.5.1/lib/sfml-system-s-d", 
		"./deps/SFML-2.5.1/lib/sfml-network-s-d", 
		"./deps/SFML-2.5.1/lib/sfml-main-d", 
		"./deps/SFML-2.5.1/lib/freetype", 

		"opengl32",	"winmm", "gdi32", "user32",
		"ws2_32", "Advapi32"}
        targetdir ("bin/Debug")
        targetsuffix "_d"
        objdir ("build/" .. "Debug")
        symbols "On"

    configuration "Release"
		links { "./deps/SFML-2.5.1/lib/sfml-graphics-s", 
		"./deps/SFML-2.5.1/lib/sfml-window-s", 
		"./deps/SFML-2.5.1/lib/sfml-system-s", 
		"./deps/SFML-2.5.1/lib/sfml-network-s", 
		"./deps/SFML-2.5.1/lib/freetype", 

		"opengl32",	"winmm", "gdi32", "user32","Advapi32"}
        targetdir ("bin/Release")
		targetsuffix "_r"
        objdir ("build/" .. "Release")
        optimize "On"

	end

	project "Pong_Client"
		files {
      "./src/MainClient.cpp",
      "./src/ClientPongApp.cpp",
      "./include/ClientPongApp.h",
	  "./src/Player.cpp",
      "./include/Player.h",
	  "./src/Ball.cpp",
      "./include/Ball.h"
    }


	project "Pong_Server"
		files {
	  "./src/MainServer.cpp",
      "./src/ServerPongApp.cpp",
      "./include/ServerPongApp.h",
	  "./src/Player.cpp",
      "./include/Player.h",
	  "./src/Ball.cpp",
      "./include/Ball.h"
		}
