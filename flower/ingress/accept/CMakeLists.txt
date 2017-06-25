add_executable(flower_ingress_accept
	flower_service.h
	flower/ingress/accept/accept.cc
)
target_link_libraries(flower_ingress_accept arpc argdata)
