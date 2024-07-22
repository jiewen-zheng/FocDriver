cmake_minimum_required(VERSION 3.26)
project(ArduinoAPI)

set(CMAKE_CXX_STANDARD 14)

include_directories(../)

file(GLOB_RECURSE SOURCES
        "main.cpp"
        "../soft_timer.c"
)


add_executable(${PROJECT_NAME} ${SOURCES})