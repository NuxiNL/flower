cmake_minimum_required(VERSION 3.8.0)

project(flower)

include(CheckPrototypeDefinition)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build test programs, using Google Test" ON)

find_package(Threads REQUIRED)

check_prototype_definition(program_main
	"void program_main(const argdata_t *ad)"
	""
	"program.h"
	HAVE_PROGRAM_MAIN)
if(HAVE_PROGRAM_MAIN)
	set(MAIN_SOURCEFILE "program_main.cc")
else()
	set(MAIN_SOURCEFILE "main.cc")
endif()

set(APROTOC_NAME "aprotoc" CACHE STRING "Name of the aprotoc command")
find_program(APROTOC_COMMAND ${APROTOC_NAME})
mark_as_advanced(APROTOC_COMMAND)
if(NOT APROTOC_COMMAND)
	message(FATAL_ERROR "Could not find aprotoc, set APROTOC_NAME")
endif()

function(add_aprotoc base)
	get_filename_component(base_dir ${base} DIRECTORY)
	add_custom_command(
		OUTPUT ${base}.ad.h
		COMMAND mkdir -p ${CMAKE_BINARY_DIR}/${base_dir} && ${APROTOC_COMMAND} <${CMAKE_SOURCE_DIR}/${base}.proto >${CMAKE_BINARY_DIR}/${base}.ad.h
		DEPENDS ${CMAKE_SOURCE_DIR}/${base}.proto
	)

	install(
		FILES ${base}.proto ${CMAKE_BINARY_DIR}/${base}.ad.h
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${base_dir}
	)
endfunction()

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

include(flower/cat/CMakeLists.txt)
include(flower/egress/connect/CMakeLists.txt)
include(flower/ingress/accept/CMakeLists.txt)
include(flower/protocol/CMakeLists.txt)
include(flower/resolver/getaddrinfo/CMakeLists.txt)
include(flower/switchboard/CMakeLists.txt)

if(BUILD_TESTS)
  add_subdirectory(contrib/googletest-release-1.8.0/googletest EXCLUDE_FROM_ALL)
  include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
  include(unittests/CMakeLists.txt)
endif(BUILD_TESTS)
