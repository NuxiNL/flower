set(APROTOC_NAME "aprotoc" CACHE STRING "Name of the aprotoc command")
find_program(APROTOC_COMMAND ${APROTOC_NAME})
mark_as_advanced(APROTOC_COMMAND)
if(NOT APROTOC_COMMAND)
	message(FATAL_ERROR "Could not find aprotoc, set APROTOC_NAME")
endif()

function(add_aprotoc base)
	add_custom_command(
		OUTPUT ${base}.ad.h
		COMMAND mkdir -p ${CMAKE_BINARY_DIR}/`dirname ${base}` && ${APROTOC_COMMAND} <${CMAKE_SOURCE_DIR}/${base}.proto >${CMAKE_BINARY_DIR}/${base}.ad.h
		DEPENDS ${CMAKE_SOURCE_DIR}/${base}.proto
	)
endfunction()

add_aprotoc(flower/protocol/egress)
add_aprotoc(flower/protocol/resolver)
add_aprotoc(flower/protocol/server)
add_aprotoc(flower/protocol/switchboard)

install(
	FILES
		flower/protocol/egress.proto
		flower/protocol/resolver.proto
		flower/protocol/server.proto
		flower/protocol/switchboard.proto
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/flower/protocol
)
