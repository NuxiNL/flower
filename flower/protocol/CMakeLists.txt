set(APROTOC_NAME "aprotoc" CACHE STRING "Name of the aprotoc command")
find_program(APROTOC_COMMAND ${APROTOC_NAME})
mark_as_advanced(APROTOC_COMMAND)
if(NOT APROTOC_COMMAND)
	message(FATAL_ERROR "Could not find aprotoc, set APROTOC_NAME")
endif()

add_custom_command(
	OUTPUT flower/protocol/egress.ad.h
	COMMAND mkdir -p ${CMAKE_BINARY_DIR}/flower/protocol && ${APROTOC_COMMAND} <${CMAKE_SOURCE_DIR}/flower/protocol/egress.proto >${CMAKE_BINARY_DIR}/flower/protocol/egress.ad.h
	DEPENDS ${CMAKE_SOURCE_DIR}/flower/protocol/egress.proto
)
add_custom_command(
	OUTPUT flower/protocol/resolver.ad.h
	COMMAND mkdir -p ${CMAKE_BINARY_DIR}/flower/protocol && ${APROTOC_COMMAND} <${CMAKE_SOURCE_DIR}/flower/protocol/resolver.proto >${CMAKE_BINARY_DIR}/flower/protocol/resolver.ad.h
	DEPENDS ${CMAKE_SOURCE_DIR}/flower/protocol/resolver.proto
)
add_custom_command(
	OUTPUT flower/protocol/server.ad.h
	COMMAND mkdir -p ${CMAKE_BINARY_DIR}/flower/protocol && ${APROTOC_COMMAND} <${CMAKE_SOURCE_DIR}/flower/protocol/server.proto >${CMAKE_BINARY_DIR}/flower/protocol/server.ad.h
	DEPENDS ${CMAKE_SOURCE_DIR}/flower/protocol/server.proto
)
add_custom_command(
	OUTPUT flower/protocol/switchboard.ad.h
	COMMAND mkdir -p ${CMAKE_BINARY_DIR}/flower/protocol && ${APROTOC_COMMAND} <${CMAKE_SOURCE_DIR}/flower/protocol/switchboard.proto >${CMAKE_BINARY_DIR}/flower/protocol/switchboard.ad.h
	DEPENDS ${CMAKE_SOURCE_DIR}/flower/protocol/switchboard.proto
)

install(
	FILES
		flower/protocol/egress.proto
		flower/protocol/resolver.proto
		flower/protocol/server.proto
		flower/protocol/switchboard.proto
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/flower/proto
)
