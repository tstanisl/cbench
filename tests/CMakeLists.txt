set(CMAKE_C_STANDARD 99)

add_executable (sleep_test sleep.c)
target_link_libraries(sleep_test PUBLIC cbench)


