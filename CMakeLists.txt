cmake_minimum_required(VERSION 3.16)
project(gstreamer_cpp_pipeline CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find GStreamer via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0)

add_executable(pipelineApp
    src/main.cpp
    src/Pipeline.cpp
)

target_include_directories(pipelineApp PRIVATE
    include
    ${GST_INCLUDE_DIRS}
)

target_link_directories(pipelineApp PRIVATE
    ${GST_LIBRARY_DIRS}
)

target_link_libraries(pipelineApp PRIVATE
    ${GST_LIBRARIES}
)