add_custom_command(OUTPUT flower/proto/egress.h
	COMMAND mkdir -p ${CMAKE_BINARY_DIR}/flower/proto && ${APROTOC_COMMAND} <${CMAKE_SOURCE_DIR}/flower/proto/egress.proto >${CMAKE_BINARY_DIR}/flower/proto/egress.h
	DEPENDS ${CMAKE_SOURCE_DIR}/flower/proto/egress.proto
)
add_custom_command(OUTPUT flower/proto/resolver.h
	COMMAND mkdir -p ${CMAKE_BINARY_DIR}/flower/proto && ${APROTOC_COMMAND} <${CMAKE_SOURCE_DIR}/flower/proto/resolver.proto >${CMAKE_BINARY_DIR}/flower/proto/resolver.h
	DEPENDS ${CMAKE_SOURCE_DIR}/flower/proto/resolver.proto
)
add_custom_command(OUTPUT flower/proto/switchboard.h
	COMMAND mkdir -p ${CMAKE_BINARY_DIR}/flower/proto && ${APROTOC_COMMAND} <${CMAKE_SOURCE_DIR}/flower/proto/switchboard.proto >${CMAKE_BINARY_DIR}/flower/proto/switchboard.h
	DEPENDS ${CMAKE_SOURCE_DIR}/flower/proto/switchboard.proto
)
