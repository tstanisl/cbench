cmake_minimum_required (VERSION 2.8.11)
project (cbench)
set(CMAKE_C_STANDARD 99)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_library(cbench STATIC
)

target_include_directories(cbench PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(cbench PUBLIC)

add_subdirectory(tests)

