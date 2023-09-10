cmake_minimum_required(VERSION 3.12)

project(RenderingCookbook CXX C)

option(BUILD_WITH_EASY_PROFILER "Enable EasyProfiler usage" ON)
option(BUILD_WITH_OPTICK "Enable Optick usage" OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(WIN32)
	set(PYTHON_EXECUTABLE "python")
else()
	set(PYTHON_EXECUTABLE "python3")
endif()

if((NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/.bootstrap.json") OR ("${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt" IS_NEWER_THAN "${CMAKE_CURRENT_SOURCE_DIR}/deps/.bootstrap.json"))
	execute_process(
		COMMAND ${PYTHON_EXECUTABLE} "bootstrap.py"
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		RESULT_VARIABLE bootstrap_result
	)
	if (NOT (${bootstrap_result} EQUAL 0))
		message(FATAL_ERROR "Cannot run bootstrap.py")
	endif()
endif()

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")

set(EASY_PROFILER_NO_GUI ON CACHE BOOL "")
set(EASY_PROFILER_NO_SAMPLES ON CACHE BOOL "")

set(ASSIMP_NO_EXPORT ON CACHE BOOL "")
set(ASSIMP_BUILD_DRACO OFF CACHE BOOL "")
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "")
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")
set(ASSIMP_INSTALL_PDB OFF CACHE BOOL "")
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "")
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "")
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "")

set(MESHOPT_BUILD_DEMO OFF CACHE BOOL "")
set(MESHOPT_BUILD_TOOLS OFF CACHE BOOL "")
set(MESHOPT_BUILD_SHARED_LIBS OFF CACHE BOOL "")

add_subdirectory(deps/src/etc2comp)

if(WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

#include_directories(deps/src/vulkan/include)

add_subdirectory(deps/src/glfw)
if(BUILD_WITH_OPTICK)
add_subdirectory(deps/src/optick)
endif()
add_subdirectory(deps/src/easy_sources)
add_subdirectory(deps/src/assimp)
add_subdirectory(deps/src/meshoptimizer)


include_directories(.)
include_directories(deps/src)
include_directories(deps/src/glfw/include)
include_directories(deps/src/glm)
include_directories(deps/src/gli)
include_directories(deps/glad/include)
include_directories(deps/src/assimp/include)
include_directories(deps/src/etc2comp/EtcLib/Etc)
include_directories(deps/src/etc2comp/EtcLib/EtcCodec)
include_directories(deps/src/glslang/glslang/Include)
include_directories(deps/src/taskflow)
include_directories(deps/src/rapidjson/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/deps/src/assimp/include)
include_directories(deps/src/stb)

set(GLAD_SOURCES deps/glad/src/glad.c)

add_library(glad ${GLAD_SOURCES})

if(BUILD_WITH_OPTICK)
	message("Enabled Optick")
	add_definitions(-DBUILD_WITH_OPTICK=1)
	set_property(TARGET OptickCore PROPERTY FOLDER "ThirdPartyLibraries")
endif()

set_property(TARGET glfw          PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET assimp        PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET EtcLib        PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET EtcTool       PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET glad          PROPERTY FOLDER "ThirdPartyLibraries")
#set_property(TARGET IrrXML        PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET meshoptimizer PROPERTY FOLDER "ThirdPartyLibraries")
if(WIN32)
#set_property(TARGET zlib          PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET zlibstatic    PROPERTY FOLDER "ThirdPartyLibraries")
set_property(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTY FOLDER "ThirdPartyLibraries")
endif()
set_property(TARGET uninstall     PROPERTY FOLDER "ThirdPartyLibraries")



add_subdirectory(Chapter1/02_CMake)

add_subdirectory(Chapter2/01_GLFW)
add_subdirectory(Chapter2/02_GLM)
add_subdirectory(Chapter2/03_STB)
add_subdirectory(Chapter2/04_ImGui)
add_subdirectory(Chapter2/05_EasyProfiler)
if(BUILD_WITH_OPTICK)
add_subdirectory(Chapter2/06_Optick)
endif()
add_subdirectory(Chapter2/07_Assimp)
add_subdirectory(Chapter2/08_ETC2Comp)
add_subdirectory(Chapter2/09_Taskflow)
add_subdirectory(Chapter2/10_MeshOptimizer)

