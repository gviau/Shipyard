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
	common/vertexformat.cpp
)

set (SHIPYARD_COMMON_HEADER_FILES
	common/common.h
	common/vertexformat.h
)

set (SHIPYARD_COMMON_WRAPPER_SOURCE_FILES
	common/wrapper/buffer.cpp
	common/wrapper/renderdevice.cpp
	common/wrapper/renderdevicecontext.cpp
	common/wrapper/shader.cpp
	common/wrapper/texture.cpp
	common/wrapper/viewsurface.cpp
)

set (SHIPYARD_COMMON_WRAPPER_HEADER_FILES
	common/wrapper/buffer.h
	common/wrapper/renderdevice.h
	common/wrapper/renderdevicecontext.h
	common/wrapper/shader.h
	common/wrapper/texture.h
	common/wrapper/viewsurface.h
	common/wrapper/wrapper.h
	common/wrapper/wrapper_common.h
)

set (SHIPYARD_COMMON_WRAPPER_DX11_SOURCE_FILES
	common/wrapper/dx11/dx11_common.cpp
	common/wrapper/dx11/dx11buffer.cpp
	common/wrapper/dx11/dx11renderdevice.cpp
	common/wrapper/dx11/dx11renderdevicecontext.cpp
	common/wrapper/dx11/dx11shader.cpp
	common/wrapper/dx11/dx11texture.cpp
	common/wrapper/dx11/dx11viewsurface.cpp
)

set (SHIPYARD_COMMON_WRAPPER_DX11_HEADER_FILES
	common/wrapper/dx11/dx11_common.h
	common/wrapper/dx11/dx11buffer.h
	common/wrapper/dx11/dx11renderdevice.h
	common/wrapper/dx11/dx11renderdevicecontext.h
	common/wrapper/dx11/dx11shader.h
	common/wrapper/dx11/dx11texture.h
	common/wrapper/dx11/dx11viewsurface.h
)

set (SHIPYARD_SYSTEM_SOURCE_FILES
	system/array.cpp
	system/memory.cpp
	system/string.cpp
)

set (SHIPYARD_SYSTEM_HEADER_FILES
	system/array.h
	system/memory.h
	system/platform.h
	system/string.h
)

source_group("Source Files\\common\\" FILES ${SHIPYARD_COMMON_SOURCE_FILES})
source_group("Header Files\\common\\" FILES ${SHIPYARD_COMMON_HEADER_FILES})

source_group("Source Files\\common\\wrapper\\" FILES ${SHIPYARD_COMMON_WRAPPER_SOURCE_FILES})
source_group("Header Files\\common\\wrapper\\" FILES ${SHIPYARD_COMMON_WRAPPER_HEADER_FILES})

source_group("Source Files\\common\\wrapper\\dx11\\" FILES ${SHIPYARD_COMMON_WRAPPER_DX11_SOURCE_FILES})
source_group("Header Files\\common\\wrapper\\dx11\\" FILES ${SHIPYARD_COMMON_WRAPPER_DX11_HEADER_FILES})

source_group("Source Files\\system\\" FILES ${SHIPYARD_SYSTEM_SOURCE_FILES})
source_group("Header Files\\system\\" FILES ${SHIPYARD_SYSTEM_HEADER_FILES})

include_directories (extern)
include_directories (./)

add_definitions(-DSHIPYARD_NONCLIENT_BUILD)

add_library(
    shipyard
	
	SHARED
	
    ${EXTERN_IMGUI_SOURCE_FILES}
	${EXTERN_GLM_SOURCE_FILES}
	${SHIPYARD_COMMON_SOURCE_FILES}
	${SHIPYARD_COMMON_WRAPPER_SOURCE_FILES}
	${SHIPYARD_COMMON_WRAPPER_DX11_SOURCE_FILES}
	${SHIPYARD_SYSTEM_SOURCE_FILES}
	
    ${EXTERN_IMGUI_HEADER_FILES}
	${EXTERN_GLM_HEADER_FILES}
	${SHIPYARD_COMMON_HEADER_FILES}
	${SHIPYARD_COMMON_WRAPPER_HEADER_FILES}
	${SHIPYARD_COMMON_WRAPPER_DX11_HEADER_FILES}
	${SHIPYARD_SYSTEM_HEADER_FILES}
)

target_link_libraries(
    shipyard

	${DirectX_D3D11_LIBRARIES}
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