cmake_minimum_required(VERSION 3.28.1)
project(run)

set(CMAKE_CXX_STANDARD 23)

add_executable(run main.cpp)

set(CMAKE_CXX_FLAGS_RELEASE /O2)

# Disable CRT
target_compile_options(run PRIVATE /Gm- /GR- /EHa- /O2)
target_link_options(run PRIVATE /NODEFAULTLIB /ENTRY:mainCRTStartup /subsystem:windows)

project(run_installer)

add_executable(run_installer installer.cpp)

set(CMAKE_CXX_FLAGS_RELEASE /Ot)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GS- /RTC-")

# Disable CRT
target_compile_options(run_installer PRIVATE /Gm- /GR- /EHa-)
target_link_options(run_installer PRIVATE /NODEFAULTLIB /ENTRY:mainCRTStartup /subsystem:windows)