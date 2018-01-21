add_aprotoc(flower/cat/configuration)

add_executable(
	flower_cat
	flower/cat/${MAIN_SOURCEFILE}
	flower/cat/configuration.ad.h
	flower/cat/start.cc
	flower/protocol/server.ad.h
	flower/protocol/switchboard.ad.h
)

target_link_libraries(
	flower_cat
	Threads::Threads
	argdata
	arpc
	${JSONCPP_LIBRARIES}
)
target_include_directories(
	flower_cat
	PUBLIC ${JSONCPP_INCLUDE_DIRS}
)
target_compile_options(
	flower_cat
	PUBLIC ${JSONCPP_CFLAGS_OTHER}
)

install(
	TARGETS flower_cat
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)
