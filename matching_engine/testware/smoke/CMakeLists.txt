add_executable(smoke smoke.cpp)
target_link_libraries(smoke gtest_main)
target_link_libraries(smoke matching)

add_test(NAME no_smoke_without_fire COMMAND smoke)
