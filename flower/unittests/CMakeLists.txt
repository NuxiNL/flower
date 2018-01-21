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
	${JSONCPP_LIBRARIES}
)
target_include_directories(
	flower_unittests
	PUBLIC ${JSONCPP_INCLUDE_DIRS}
)
target_compile_options(
	flower_unittests
	PUBLIC ${JSONCPP_CFLAGS_OTHER}
)
