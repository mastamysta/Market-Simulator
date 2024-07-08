add_executable(combine_ticks combine_ticks.cpp)

target_include_directories(combine_ticks PRIVATE include)

target_link_libraries (combine_ticks PUBLIC logging)
