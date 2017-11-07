cmake_minimum_required (VERSION 2.8)

set (
    CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    "${CMAKE_SOURCE_DIR}/CMakeModules/Packages"
)

find_package (DirectX)

message(STATUS "DirectX found? ${DirectX_D3D11_FOUND}")
if (DirectX_FOUND)
	message (STATUS "DirectX include: ${DirectX_INCLUDE_DIR}")
	message (STATUS "DirectX libs: ${DirectX_D3D11_LIBRARIES}")
endif ()

find_package(Assimp)

message(STATUS "Assimp found? ${ASSIMP_FOUND}")
if (ASSIMP_FOUND)
	message (STATUS "Assimp include: ${ASSIMP_INCLUDE_DIR}")
	message (STATUS "Assimp libs: ${ASSIMP_LIBRARY}")
else ()
	set (MISSING_MODULES ${MISSING_MODULES} "Assimp")
endif ()

find_package (FreeImage)

message (STATUS "FreeImage found? ${FreeImage_FOUND}")
if (FreeImage_FOUND)
	message (STATUS "FreeImage include: ${FreeImage_INCLUDE_DIRS}")
	message (STATUS "FreeImage libs: ${FreeImage_LIBRARIES}")
else ()
	set (MISSING_MODULES ${MISSING_MODULES} "FreeImage")
endif ()

if (MISSING_MODULES)
    message(FATAL_ERROR "Could generate all makefiles: missing modules: ${MISSING_MODULES}")
endif (MISSING_MODULES)

add_subdirectory (shipyard)
add_subdirectory (shipyard-viewer)