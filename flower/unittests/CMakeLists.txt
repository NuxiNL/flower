add_executable(
	flower_unittests
	flower/switchboard/handle_test.cc
)

target_link_libraries(
	flower_unittests
	flower_switchboard_lib
	arpc
	argdata
	gtest_main
	jsoncpp
)
