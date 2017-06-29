add_library(
	flower_switchboard_lib
	flower/switchboard/directory.cc
	flower/switchboard/egress_listener.cc
	flower/switchboard/handle.cc
	flower/switchboard/label_map.cc
	flower/switchboard/listener.cc
	flower/switchboard/server_listener.cc
	flower/protocol/egress.ad.h
	flower/protocol/resolver.ad.h
	flower/protocol/server.ad.h
	flower/protocol/switchboard.ad.h
)

add_executable(
	flower_switchboard
	flower/switchboard/${MAIN_SOURCEFILE}
)

target_link_libraries(
	flower_switchboard
	flower_switchboard_lib
	Threads::Threads
	argdata
	arpc
)

install(
	TARGETS flower_switchboard
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_executable(
	flower_switchboard_test
	flower/switchboard/handle_test.cc
)

target_link_libraries(
	flower_switchboard_test
	flower_switchboard_lib
	arpc
	argdata
	gtest_main
)
