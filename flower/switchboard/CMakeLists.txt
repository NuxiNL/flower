add_executable(flower_switchboard
	flower/switchboard/${MAIN_SOURCEFILE}
	flower/switchboard/directory.cc
	flower/switchboard/egress_listener.cc
	flower/switchboard/handle.cc
	flower/switchboard/label_map.cc
	flower/switchboard/server_listener.cc
)
target_link_libraries(flower_switchboard arpc argdata)
