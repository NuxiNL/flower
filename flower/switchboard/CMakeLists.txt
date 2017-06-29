add_executable(flower_switchboard
	flower/switchboard/${MAIN_SOURCEFILE}
	flower/switchboard/directory.cc
	flower/switchboard/egress_listener.cc
	flower/switchboard/handle.cc
	flower/switchboard/label_map.cc
	flower/switchboard/listener.cc
	flower/switchboard/server_listener.cc
	flower/proto/egress.h
	flower/proto/resolver.h
	flower/proto/server.h
	flower/proto/switchboard.h
)
target_link_libraries(flower_switchboard arpc argdata Threads::Threads)
