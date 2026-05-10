enable_testing()

# Try to find GTest, but don't fail if not found
find_package(GTest QUIET)

if(GTest_FOUND)
    add_executable(voxel_tests test_main.cpp)
    target_link_libraries(voxel_tests GTest::gtest_main)
    add_test(NAME voxel_tests COMMAND voxel_tests)
    
    add_executable(gamepad_tests test_gamepad.cpp ../src/platform/gamepad.cpp)
    target_include_directories(gamepad_tests PRIVATE ../src ../include /usr/include /usr/local/include)
    target_link_libraries(gamepad_tests GTest::gtest_main pthread dl)
    add_test(NAME gamepad_tests COMMAND gamepad_tests)
else()
    message(STATUS "GTest not found - tests will not be built")
endif()
