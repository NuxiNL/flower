if(NOT HAVE_PROGRAM_MAIN)
	add_aprotoc(flower/egress/connect/configuration)

	add_executable(
		flower_egress_connect
		flower/egress/connect/${MAIN_SOURCEFILE}
		flower/egress/connect/configuration.ad.h
		flower/egress/connect/start.cc
		flower/protocol/egress.ad.h
		flower/protocol/switchboard.ad.h
	)

	target_link_libraries(
		flower_egress_connect
		argdata
		arpc
		${JSONCPP_LIBRARIES}
	)
	target_include_directories(
		flower_egress_connect
		PUBLIC ${JSONCPP_INCLUDE_DIRS}
	)
	target_compile_options(
		flower_egress_connect
		PUBLIC ${JSONCPP_CFLAGS_OTHER}
	)

	install(
		TARGETS flower_egress_connect
		DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
endif()
