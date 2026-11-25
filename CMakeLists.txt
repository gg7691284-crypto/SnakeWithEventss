cmake_minimum_required(VERSION 3.16)
project(SnakeWithEvents LANGUAGES CXX)

add_executable(snake snake.cpp)
target_compile_features(snake PRIVATE cxx_std_17)

include(CTest)
if(BUILD_TESTING)
    add_executable(snake_tests tests.cpp)
    target_compile_features(snake_tests PRIVATE cxx_std_17)
    add_test(NAME game_rules COMMAND snake_tests)
    add_test(NAME demo COMMAND snake --demo)
endif()
