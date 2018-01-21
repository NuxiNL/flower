add_aprotoc(flower/ls/configuration)

add_executable(
	flower_ls
	flower/ls/${MAIN_SOURCEFILE}
	flower/ls/configuration.ad.h
	flower/ls/start.cc
	flower/protocol/switchboard.ad.h
)

target_link_libraries(
	flower_ls
	argdata
	arpc
	${JSONCPP_LIBRARIES}
)
target_include_directories(
	flower_ls
	PUBLIC ${JSONCPP_INCLUDE_DIRS}
)
target_compile_options(
	flower_ls
	PUBLIC ${JSONCPP_CFLAGS_OTHER}
)

install(
	TARGETS flower_ls
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)
