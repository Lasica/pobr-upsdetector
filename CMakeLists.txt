cmake_minimum_required(VERSION 3.0)

project(upsdetector)

add_executable(upsdetector main.cpp)

install(TARGETS upsdetector RUNTIME DESTINATION bin)
