cmake_minimum_required(VERSION 3.28.1)
project(run)

set(CMAKE_CXX_STANDARD 23)

add_executable(run main.cpp)

set(CMAKE_CXX_FLAGS_RELEASE /O2)

# Disable CRT
target_compile_options(run PRIVATE /Gm- /GR- /EHa- /O2)
target_link_options(run PRIVATE /NODEFAULTLIB /ENTRY:mainCRTStartup /subsystem:windows)
