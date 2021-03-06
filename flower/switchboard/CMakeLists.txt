add_library(
	flower_switchboard_lib
	flower/protocol/egress.ad.h
	flower/protocol/resolver.ad.h
	flower/protocol/server.ad.h
	flower/protocol/switchboard.ad.h
	flower/switchboard/configuration.ad.h
	flower/switchboard/directory.cc
	flower/switchboard/egress_listener.cc
	flower/switchboard/handle.cc
	flower/switchboard/server_listener.cc
	flower/switchboard/start.cc
	flower/switchboard/target_picker.cc
	flower/switchboard/worker_pool.cc
)

add_aprotoc(flower/switchboard/configuration)

target_include_directories(
	flower_switchboard_lib
	PUBLIC ${JSONCPP_INCLUDE_DIRS}
)
target_compile_options(
	flower_switchboard_lib
	PUBLIC ${JSONCPP_CFLAGS_OTHER}
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
	${JSONCPP_LIBRARIES}
)

install(
	TARGETS flower_switchboard
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)
