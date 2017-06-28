add_executable(flower_switchboard
	flower/switchboard/${MAIN_SOURCEFILE}
	flower/switchboard/handle.cc
)
target_link_libraries(flower_switchboard arpc argdata)
