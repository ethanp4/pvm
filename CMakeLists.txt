#cmake -S . -B build -G Ninja
cmake_minimum_required(VERSION 3.20)
project(pvm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(pvm main.cpp)

target_compile_options(pvm PRIVATE -Werror=return-type)

add_custom_target(launch
	COMMAND $<TARGET_FILE:pvm>
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	VERBATIM)
add_dependencies(launch pvm)