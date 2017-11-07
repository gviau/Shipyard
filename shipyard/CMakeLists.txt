cmake_minimum_required(VERSION 2.8)

project(shipyard)

set(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/lib/")

# Extern files	
file (GLOB_RECURSE EXTERN_IMGUI_SOURCE_FILES extern/imgui/*.cpp)
file (GLOB_RECURSE EXTERN_IMGUI_HEADER_FILES extern/imgui/*.h)

source_group("Source Files\\extern\\imgui" FILES ${EXTERN_IMGUI_SOURCE_FILES})
source_group("Header Files\\extern\\imgui" FILES ${EXTERN_IMGUI_HEADER_FILES})

file (GLOB_RECURSE EXTERN_GLM_SOURCE_FILES "extern/glm/*.cpp" "extern/glm/*.inl")
file (GLOB_RECURSE EXTERN_GLM_HEADER_FILES "extern/glm/*.h" "extern/glm/*.hpp")

source_group("Source Files\\extern\\glm" FILES ${EXTERN_GLM_SOURCE_FILES})
source_group("Header Files\\extern\\glm" FILES ${EXTERN_GLM_HEADER_FILES})

# Shipyard DLL
set (SHIPYARD_COMMON_SOURCE_FILES
	common/dummy.cpp
)

set (SHIPYARD_COMMON_HEADER_FILES
	common/dummy.h
)

source_group("Source Files\\common\\" FILES ${SHIPYARD_COMMON_SOURCE_FILES})
source_group("Header Files\\common\\" FILES ${SHIPYARD_COMMON_HEADER_FILES})

include_directories (extern)
include_directories (./)

add_definitions(-DSHIPYARD_NONCLIENT_BUILD)

add_library(
    shipyard
	
	SHARED
	
    ${EXTERN_IMGUI_SOURCE_FILES}
	${EXTERN_GLM_SOURCE_FILES}
	${SHIPYARD_COMMON_SOURCE_FILES}
	
    ${EXTERN_IMGUI_HEADER_FILES}
	${EXTERN_GLM_HEADER_FILES}
	${SHIPYARD_COMMON_HEADER_FILES}
)

# Add a bunch of post build events to copy the required files in the bin folder
add_custom_command(TARGET shipyard POST_BUILD COMMAND
	${CMAKE_COMMAND} -E make_directory
	"${CMAKE_SOURCE_DIR}/bin/$<CONFIGURATION>/"
)

add_custom_command(TARGET shipyard POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${CMAKE_SOURCE_DIR}/lib/$<CONFIGURATION>/shipyard.dll"
	"${CMAKE_SOURCE_DIR}/bin/$<CONFIGURATION>/"
)