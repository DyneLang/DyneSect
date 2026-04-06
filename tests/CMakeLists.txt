include(GoogleTest)

# --- dynesect tests -----------------------------------------------------
add_executable(test_dynesect test_dynesect.cpp)
target_link_libraries(test_dynesect PRIVATE GTest::gtest_main)
gtest_discover_tests(test_dynesect)

# --- dynediff tests -----------------------------------------------------
add_executable(test_dynediff test_dynediff.cpp)
target_link_libraries(test_dynediff PRIVATE GTest::gtest_main)
gtest_discover_tests(test_dynediff)

# --- dynepatch tests ----------------------------------------------------
add_executable(test_dynepatch test_dynepatch.cpp)
target_link_libraries(test_dynepatch PRIVATE GTest::gtest_main)
gtest_discover_tests(test_dynepatch)
