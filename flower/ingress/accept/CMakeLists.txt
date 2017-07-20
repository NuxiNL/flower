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
	jsoncpp
)

install(
	TARGETS flower_ingress_accept
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)
