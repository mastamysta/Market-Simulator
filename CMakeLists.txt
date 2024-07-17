cmake_minimum_required(VERSION 3.14)
enable_testing()
project (Market-Simulator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Logging header files
add_subdirectory(logging)

# Some tools to combine CSV datastreams
add_subdirectory(tickdata_util)

# A program to take in historical order data and order feed data from simlated market participants
# and maintain an order book.
add_subdirectory(sim)

# A matching engine
add_subdirectory(matching_engine)

# Externals
add_subdirectory(extern)
