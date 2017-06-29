add_executable(
	flower_egress_connect
	flower/egress/connect/connect.cc
	flower/proto/egress.ad.h
	flower/proto/switchboard.ad.h
)

target_link_libraries(
	flower_egress_connect
	argdata
	arpc
)

install(
	TARGETS flower_egress_connect
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)
