set(CMAKE_OSX_ARCHITECTURES "x86_64")

add_library(ProcessorTracer ProcessorTracer.cpp)

target_compile_options(ProcessorTracer PUBLIC -g -Wall)
target_include_directories(ProcessorTracer PUBLIC "${PROJECT_SOURCE_DIR}/include")
#target_link_libraries(ProcessorTracer ipt)