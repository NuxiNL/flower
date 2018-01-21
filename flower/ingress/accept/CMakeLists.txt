if(NOT HAVE_PROGRAM_MAIN)
	add_aprotoc(flower/ingress/accept/configuration)

	add_executable(
		flower_ingress_accept
		flower/ingress/accept/${MAIN_SOURCEFILE}
		flower/ingress/accept/configuration.ad.h
		flower/ingress/accept/start.cc
		flower/protocol/switchboard.ad.h
	)

	target_link_libraries(
		flower_ingress_accept
		argdata
		arpc
		${JSONCPP_LIBRARIES}
	)
	target_include_directories(
		flower_ingress_accept
		PUBLIC ${JSONCPP_INCLUDE_DIRS}
	)
	target_compile_options(
		flower_ingress_accept
		PUBLIC ${JSONCPP_CFLAGS_OTHER}
	)

	install(
		TARGETS flower_ingress_accept
		DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
endif()
