project(abc)

cmake_minimum_required(VERSION 2.6)

include_directories(..)

SET(EXE_NAME example1 example2 example3 example4)

foreach (exe ${EXE_NAME})
    add_executable(${exe} ${exe}.cpp ../tinyxml2.cpp)
endforeach()
