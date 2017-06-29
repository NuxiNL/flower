add_executable(
	flower_resolver_getaddrinfo
	flower/protocol/resolver.ad.h
	flower/protocol/switchboard.ad.h
	flower/resolver/getaddrinfo/getaddrinfo.cc
)

target_link_libraries(
	flower_resolver_getaddrinfo
	argdata
	arpc
)

install(
	TARGETS flower_resolver_getaddrinfo
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)
