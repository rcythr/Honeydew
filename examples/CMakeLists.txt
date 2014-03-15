# This file is part of Honeydew
# Honeydew is licensed under the MIT LICENSE. See the LICENSE file for more info.

add_executable(round_robin round_robin.cc)
add_executable(round_robin_priority round_robin_priority.cc)
add_executable(least_busy least_busy.cc)
add_executable(least_busy_priority least_busy_priority.cc)
add_executable(conditional_test conditional_test.cc)
add_executable(pipeline_test pipeline_test.cc)
add_executable(prime_sieve prime_sieve.cc)
add_executable(timer_test timer_test.cc)

target_link_libraries(round_robin honeydew)
target_link_libraries(round_robin_priority honeydew)
target_link_libraries(least_busy honeydew)
target_link_libraries(least_busy_priority honeydew)
target_link_libraries(conditional_test honeydew)
target_link_libraries(pipeline_test honeydew)
target_link_libraries(prime_sieve honeydew)
target_link_libraries(timer_test honeydew)
