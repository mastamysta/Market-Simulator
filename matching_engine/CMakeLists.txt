add_library(matching)
target_include_directories(matching PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_subdirectory(src)

add_subdirectory(testware)
