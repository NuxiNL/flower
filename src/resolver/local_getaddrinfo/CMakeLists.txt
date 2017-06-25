add_executable(flower_local_getaddrinfo
	flower_resolver.h
	flower_switchboard.h
	src/resolver/local_getaddrinfo/local_getaddrinfo.cc
)
target_link_libraries(flower_local_getaddrinfo arpc argdata)
