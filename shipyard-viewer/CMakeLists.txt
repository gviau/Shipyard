cmake_minimum_required(VERSION 2.8)

project(shipyard)

set(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/lib/")

set (SHIPYARD_VIEWER_SOURCE_FILES
	main_win32.cpp
)

set (SHIPYARD_VIEWER_HEADER_FILES
)
source_group("Source Files\\" FILES ${SHIPYARD_VIEWER_SOURCE_FILES})
source_group("Header Files\\" FILES ${SHIPYARD_VIEWER_HEADER_FILES})

link_directories(${CMAKE_SOURCE_DIR}/lib)

add_executable(
	shipyard-viewer
	WIN32
	${SHIPYARD_VIEWER_SOURCE_FILES}
	${SHIPYARD_VIEWER_HEADER_FILES}
)

target_link_libraries(
	shipyard-viewer
	shipyard
)