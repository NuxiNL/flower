add_executable(
	flower_resolver_getaddrinfo
	flower/proto/resolver.ad.h
	flower/proto/switchboard.ad.h
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
