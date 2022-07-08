cmake_minimum_required(VERSION 3.22)
project(root2img)

set(CMAKE_CXX_STANDARD 17)

find_package(ROOT REQUIRED Core)
find_package(TinyTIFF REQUIRED)
include(${ROOT_USE_FILE})

include_directories(libtiff/TinyTIFF)

add_executable(root2img main.cpp)
target_link_libraries(root2img ${ROOT_LIBRARIES} TinyTIFF)
