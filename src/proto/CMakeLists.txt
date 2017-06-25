add_custom_command(OUTPUT flower_resolver.h
	COMMAND ${APROTOC_COMMAND} <${CMAKE_SOURCE_DIR}/src/proto/flower_resolver.proto >${CMAKE_BINARY_DIR}/flower_resolver.h
	DEPENDS ${CMAKE_SOURCE_DIR}/src/proto/flower_resolver.proto
)
add_custom_command(OUTPUT flower_switchboard.h
	COMMAND ${APROTOC_COMMAND} <${CMAKE_SOURCE_DIR}/src/proto/flower_switchboard.proto >${CMAKE_BINARY_DIR}/flower_switchboard.h
	DEPENDS ${CMAKE_SOURCE_DIR}/src/proto/flower_switchboard.proto
)
