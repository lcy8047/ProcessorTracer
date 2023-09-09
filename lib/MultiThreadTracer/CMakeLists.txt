set(CMAKE_OSX_ARCHITECTURES "x86_64")

add_library(MultiThreadTracer MultiThreadTracer.cpp)

target_compile_options(MultiThreadTracer PUBLIC -g -Wall)
target_include_directories(MultiThreadTracer PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(MultiThreadTracer ProcessorTracer Decoder)