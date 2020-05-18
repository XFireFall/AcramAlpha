cmake_minimum_required(VERSION 3.13)
project(diff)

set(CMAKE_CXX_STANDARD 11)

add_executable(diff main.cpp
					lib/Equation.cpp
					lib/Texfile.cpp
					lib/system.cpp
					lib/scriptparser.cpp
					)

