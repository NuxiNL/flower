cmake_minimum_required(VERSION 3.8.0)

project(flower)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

include (CheckPrototypeDefinition)
check_prototype_definition(program_main
	"void program_main(const argdata_t *ad)"
	""
	"program.h"
	HAVE_PROGRAM_MAIN)
if(HAVE_PROGRAM_MAIN)
	set(MAIN_SOURCEFILE "program_main.cpp")
else()
	set(MAIN_SOURCEFILE "main.cpp")
endif()

set(APROTOC_NAME "aprotoc" CACHE STRING "Name of the aprotoc command")
find_program(APROTOC_COMMAND ${APROTOC_NAME})
mark_as_advanced(APROTOC_COMMAND)

if(NOT APROTOC_COMMAND)
	message(FATAL_ERROR "Could not find aprotoc, set APROTOC_NAME")
endif()

add_custom_command(OUTPUT flower_service.h
	COMMAND ${APROTOC_COMMAND} <${CMAKE_SOURCE_DIR}/flower_service.proto >${CMAKE_BINARY_DIR}/flower_service.h
	DEPENDS ${CMAKE_SOURCE_DIR}/flower_service.proto
)

include_directories(${CMAKE_BINARY_DIR})

add_executable(flowerd
	flower_service.h
	src/${MAIN_SOURCEFILE}
	src/flower.h src/flower.cpp
)
target_link_libraries(flowerd arpc)
