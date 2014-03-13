# This file is part of RFUS (Rich's Fast Userspace Scheduling)
# RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

add_executable(round_robin round_robin.cc)
target_link_libraries(round_robin rfus)

add_executable(round_robin_priority round_robin_priority.cc)
target_link_libraries(round_robin_priority rfus)

add_executable(least_busy least_busy.cc)
target_link_libraries(least_busy rfus)

add_executable(least_busy_priority least_busy_priority.cc)
target_link_libraries(least_busy_priority rfus)

add_executable(conditional_test conditional_test.cc)
target_link_libraries(conditional_test rfus)

add_executable(exception_test exception_test.cc)
target_link_libraries(exception_test rfus)

add_executable(outcome_test outcome_test.cc)
target_link_libraries(outcome_test rfus)

add_executable(pipeline_test pipeline_test.cc)
target_link_libraries(pipeline_test rfus)

add_executable(prime_sieve prime_sieve.cc)
target_link_libraries(prime_sieve rfus)
