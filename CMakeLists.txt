cmake_minimum_required (VERSION 3.1)
project (mapsort)
set (CMAKE_CXX_STANDARD 11)

if (MSVC)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

find_package(Boost REQUIRED)

include_directories("${Boost_INCLUDE_DIRS}") 
include_directories("${PROJECT_BINARY_DIR}") 

add_executable(mapsort mapsort.cpp)

install (TARGETS mapsort DESTINATION ${CMAKE_SOURCE_DIR}/bin/)
