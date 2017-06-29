set(APROTOC_NAME "aprotoc" CACHE STRING "Name of the aprotoc command")
find_program(APROTOC_COMMAND ${APROTOC_NAME})
mark_as_advanced(APROTOC_COMMAND)
if(NOT APROTOC_COMMAND)
	message(FATAL_ERROR "Could not find aprotoc, set APROTOC_NAME")
endif()

add_custom_command(
	OUTPUT flower/proto/egress.ad.h
	COMMAND mkdir -p ${CMAKE_BINARY_DIR}/flower/proto && ${APROTOC_COMMAND} <${CMAKE_SOURCE_DIR}/flower/proto/egress.proto >${CMAKE_BINARY_DIR}/flower/proto/egress.ad.h
	DEPENDS ${CMAKE_SOURCE_DIR}/flower/proto/egress.proto
)
add_custom_command(
	OUTPUT flower/proto/resolver.ad.h
	COMMAND mkdir -p ${CMAKE_BINARY_DIR}/flower/proto && ${APROTOC_COMMAND} <${CMAKE_SOURCE_DIR}/flower/proto/resolver.proto >${CMAKE_BINARY_DIR}/flower/proto/resolver.ad.h
	DEPENDS ${CMAKE_SOURCE_DIR}/flower/proto/resolver.proto
)
add_custom_command(
	OUTPUT flower/proto/server.ad.h
	COMMAND mkdir -p ${CMAKE_BINARY_DIR}/flower/proto && ${APROTOC_COMMAND} <${CMAKE_SOURCE_DIR}/flower/proto/server.proto >${CMAKE_BINARY_DIR}/flower/proto/server.ad.h
	DEPENDS ${CMAKE_SOURCE_DIR}/flower/proto/server.proto
)
add_custom_command(
	OUTPUT flower/proto/switchboard.ad.h
	COMMAND mkdir -p ${CMAKE_BINARY_DIR}/flower/proto && ${APROTOC_COMMAND} <${CMAKE_SOURCE_DIR}/flower/proto/switchboard.proto >${CMAKE_BINARY_DIR}/flower/proto/switchboard.ad.h
	DEPENDS ${CMAKE_SOURCE_DIR}/flower/proto/switchboard.proto
)

install(
	FILES
		flower/proto/egress.proto
		flower/proto/resolver.proto
		flower/proto/server.proto
		flower/proto/switchboard.proto
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/flower/proto
)
