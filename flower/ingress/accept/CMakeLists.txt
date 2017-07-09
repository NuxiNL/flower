if(NOT HAVE_PROGRAM_MAIN)
	add_executable(
		flower_ingress_accept
		flower/protocol/switchboard.ad.h
		flower/ingress/accept/accept.cc
	)

	target_link_libraries(
		flower_ingress_accept
		argdata
		arpc
	)

	install(
		TARGETS flower_ingress_accept
		DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
endif()
