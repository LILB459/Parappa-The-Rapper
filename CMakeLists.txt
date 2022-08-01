cmake_minimum_required(VERSION 3.8)

###########
# Options #
###########

option(LTO "Enable link-time optimization" OFF)
option(MSVC_LINK_STATIC_RUNTIME "Link the static MSVC runtime library (Visual Studio only)" ON)

#########
# Setup #
#########

# Define project, source, and includes
project(
	PaperPup
	LANGUAGES    C CXX
	VERSION      0.0.1
	DESCRIPTION  "PaperPup"
	HOMEPAGE_URL "https://ckdev.org"
)

add_executable(PaperPup WIN32
	# = Client =
	"src/PaperPup.cpp"
	"src/PaperPup.h"
	"src/Engine.cpp"
	"src/Engine.h"
	"src/Pack.cpp"
	"src/Pack.h"
	
	# = Menu =
	"src/Menu/Menu.cpp"
	"src/Menu/Menu.h"

	# = Lua Controller =
	"src/LuaController.cpp"
	"src/LuaController.h"

	# = Platform =
	"src/Platform/Platform.h"
	"src/Platform/Filesystem.h"
	"src/Platform/Render.h"
	"src/Platform/Input.h"

	# = Platform Common =
	"src/Platform/Common/Mode2.h"
	"src/Platform/Common/Binary.h"
	"src/Platform/Common/IntArchive.h"
)

target_include_directories(PaperPup PRIVATE "src")
target_include_directories(PaperPup PRIVATE "lib")

##################
# Compiler flags #
##################

# Force Warnings
if(MSVC)
	target_compile_options(PaperPup PRIVATE /W3)
else()
	target_compile_options(PaperPup PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Strip release builds
if(GCC)
	set(CMAKE_C_FLAGS_RELEASE     "${CMAKE_C_FLAGS_RELEASE} -s")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
endif()

# Statically link other libraries
if(GCC)
	target_link_options(PaperPup PRIVATE "-static")
endif()

# Use LTO if requested
if(LTO)
	include(CheckIPOSupported)
	check_ipo_supported(RESULT result)
	if(result)
		set_target_properties(PaperPup PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
	endif()
endif()

########
# MSVC #
########

# This is messy, and has been replaced by CMAKE_MSVC_RUNTIME_LIBRARY,
# but that's a very recent CMake addition, so we're still doing it this way for now
if(MSVC AND MSVC_LINK_STATIC_RUNTIME)
	# Statically-link the CRT (vcpkg static libs do this)
	foreach(flag_var CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
		if(${flag_var} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
		endif()
	endforeach()
endif()

# Do some other MSVC fixes
if(MSVC)
	# Make it so source files are recognized as UTF-8 by MSVC
	target_compile_options(PaperPup PRIVATE "/utf-8")

	# Don't warn for standard C functions
	target_compile_definitions(PaperPup PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

#####################
# Determine platform #
#####################
if(WIN32)
	set(PAPERPUP_PLATFORM "Win32")
endif()

##########
# Win32  #
##########
if(PAPERPUP_PLATFORM MATCHES "Win32")
	message(STATUS "[PaperPup] Targeting Win32")

	# Compile sources
	target_sources(PaperPup PRIVATE
		# = Win32 Platform =
		"src/Platform/Win32/Win32.cpp"
		"src/Platform/Win32/Win32.h"
		"src/Platform/Win32/Filesystem.cpp"
		"src/Platform/Win32/Filesystem.h"
		"src/Platform/Win32/Render.cpp"
		"src/Platform/Win32/Render.h"
		"src/Platform/Win32/Input.cpp"
		"src/Platform/Win32/Input.h"

		# = Win32 Resources =
		"res/PaperPup.rc"
		"res/PaperPup.manifest"
	)
	
	# Set defines
	target_compile_definitions(PaperPup PRIVATE PAPERPUP_PLATFORM_WIN32)
	set(PAPERPUP_PLATFORM_WIN32 ON)
	
	# Use UTF-16 functionality where possible
	target_compile_definitions(PaperPup PRIVATE UNICODE _UNICODE)
	
	# Link Win32 libraries
	target_link_libraries(PaperPup PRIVATE d3d11 dxgi)
endif()

##########################
# Further compiler setup #
##########################

# Force strict C99 and C++17
set_target_properties(PaperPup PROPERTIES
	C_STANDARD 99
	C_STANDARD_REQUIRED ON
	C_EXTENSIONS OFF
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

# Set debug executable name
set_target_properties(PaperPup PROPERTIES DEBUG_OUTPUT_NAME "PaperPup_debug")

# Set output directory
set(BUILD_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set_target_properties(PaperPup PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIRECTORY}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BUILD_DIRECTORY}
	RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${BUILD_DIRECTORY}
	RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${BUILD_DIRECTORY}
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BUILD_DIRECTORY}
)

########
# Luau #
########

# Compile and link Luau
add_subdirectory("lib/luau" EXCLUDE_FROM_ALL)
target_link_libraries(PaperPup PRIVATE Luau.Compiler Luau.VM)
