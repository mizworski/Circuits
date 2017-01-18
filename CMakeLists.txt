cmake_minimum_required(VERSION 3.6)
project(PwZadanie3)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c err.c err.h circuit.c)
add_executable(PwZadanie3 ${SOURCE_FILES})