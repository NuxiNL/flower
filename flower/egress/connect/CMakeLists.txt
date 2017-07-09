if(NOT HAVE_PROGRAM_MAIN)
	add_executable(
		flower_egress_connect
		flower/egress/connect/connect.cc
		flower/protocol/egress.ad.h
		flower/protocol/switchboard.ad.h
	)

	target_link_libraries(
		flower_egress_connect
		argdata
		arpc
	)

	install(
		TARGETS flower_egress_connect
		DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
endif()
