cmake_minimum_required(VERSION 3.29)
project(VramDB)

find_package(Vulkan REQUIRED)
if(Vulkan_FOUND)
    include_directories(${Vulkan_INCLUDE_DIRS})
    link_directories(${Vulkan_LIBRARY_DIRS})
endif ()

set(CMAKE_CXX_STANDARD 20)

add_executable(VramDB main.cpp)

target_link_libraries(VramDB ${Vulkan_LIBRARIES})