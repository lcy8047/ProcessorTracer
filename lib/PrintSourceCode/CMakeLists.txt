set(CMAKE_OSX_ARCHITECTURES "x86_64")

add_library(PrintSourceCode PrintSourceCode.cpp Debug.cpp)

target_compile_options(PrintSourceCode PUBLIC -g -Wall)
target_include_directories(PrintSourceCode PUBLIC "${PROJECT_SOURCE_DIR}/include")
# order is matter
target_link_libraries(PrintSourceCode dwarf z)

if(APPLE)
    target_include_directories(PrintSourceCode PUBLIC /usr/local/include)
endif()