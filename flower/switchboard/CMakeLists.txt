add_executable(
	flower_switchboard
	flower/switchboard/${MAIN_SOURCEFILE}
	flower/switchboard/directory.cc
	flower/switchboard/egress_listener.cc
	flower/switchboard/handle.cc
	flower/switchboard/label_map.cc
	flower/switchboard/listener.cc
	flower/switchboard/server_listener.cc
	flower/proto/egress.ad.h
	flower/proto/resolver.ad.h
	flower/proto/server.ad.h
	flower/proto/switchboard.ad.h
)

target_link_libraries(
	flower_switchboard
	Threads::Threads
	argdata
	arpc
)

install(
	TARGETS flower_switchboard
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)
