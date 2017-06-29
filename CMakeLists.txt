cmake_minimum_required(VERSION 3.8.0)

project(flower)

include(CheckPrototypeDefinition)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

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

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

include(flower/egress/connect/CMakeLists.txt)
include(flower/ingress/accept/CMakeLists.txt)
include(flower/proto/CMakeLists.txt)
include(flower/resolver/getaddrinfo/CMakeLists.txt)
include(flower/switchboard/CMakeLists.txt)
