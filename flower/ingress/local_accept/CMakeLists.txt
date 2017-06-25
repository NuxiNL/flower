add_executable(flower_local_accept
	flower_service.h
	flower/ingress/local_accept/local_accept.cc
)
target_link_libraries(flower_local_accept arpc argdata)
