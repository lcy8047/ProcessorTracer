set(CMAKE_OSX_ARCHITECTURES "x86_64")

add_library(Decoder Debug.cpp Decoder.cpp)

target_compile_options(Decoder PUBLIC -g -Wall)
target_include_directories(Decoder PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(Decoder ipt PrintSourceCode)
