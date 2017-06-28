add_executable(flower_egress_connect
	flower/egress/connect/connect.cc
	flower/proto/egress.h
	flower/proto/switchboard.h
)
target_link_libraries(flower_egress_connect arpc argdata)
