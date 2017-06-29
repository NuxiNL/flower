add_executable(
	unittests
	flower/switchboard/handle_test.cc
)

target_link_libraries(
	unittests
	flower_switchboard_lib
	arpc
	argdata
	gtest_main
)
