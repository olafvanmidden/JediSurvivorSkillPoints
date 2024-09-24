cmake_minimum_required(VERSION 3.30)

set(PROJECT_NAME JediSurviorSkillPoints)
project(${PROJECT_NAME})

set(CMAKE_CXX_FLAGS "-static -static-libgcc -static-libstdc++")
add_executable(${PROJECT_NAME} main.cpp)
