add_executable(market market.cpp)

target_include_directories(market PRIVATE include)

target_link_libraries(market PUBLIC logging)
