add_executable(flower_local_getaddrinfo
	flower/proto/resolver.h
	flower/proto/switchboard.h
	flower/resolver/local_getaddrinfo/local_getaddrinfo.cc
)
target_link_libraries(flower_local_getaddrinfo arpc argdata)
