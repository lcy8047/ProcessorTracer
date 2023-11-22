cmake_minimum_required(VERSION 3.27)
set(CMAKE_CXX_STANDARD 17)
project( ProcessorTracerByIntelPT VERSION 1.0 )

if (UNIX AND NOT APPLE)
    add_subdirectory( lib )

    add_executable( tracer tracer.cpp )
    target_compile_options(tracer PUBLIC -g -Wall)
    target_link_libraries(tracer ProcessorTracer Decoder MultiThreadTracer)
    target_include_directories( tracer 
                                PUBLIC 
                                "${PROJECT_SOURCE_DIR}/include" )
    set_target_properties( tracer PROPERTIES
                        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

    add_executable( multithread_tracer multithread_tracer.cpp )
    target_compile_options(multithread_tracer PUBLIC -g -Wall)
    target_link_libraries(multithread_tracer ProcessorTracer Decoder MultiThreadTracer)
    target_include_directories( multithread_tracer 
                                PUBLIC 
                                "${PROJECT_SOURCE_DIR}/include" )
    set_target_properties( multithread_tracer PROPERTIES
                        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

    add_executable( bin_trace_test bin_trace_test.cpp )
    target_compile_options(bin_trace_test PUBLIC -g -Wall)
    target_link_libraries(bin_trace_test ProcessorTracer Decoder)
    target_include_directories( bin_trace_test 
                                PUBLIC 
                                "${PROJECT_SOURCE_DIR}/include" )
    set_target_properties( bin_trace_test PROPERTIES
                        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

    add_executable( decode_tool decode_tool.cpp )
    target_compile_options(decode_tool PUBLIC -g -Wall)
    target_link_libraries(decode_tool Decoder)
    target_include_directories( decode_tool 
                                PUBLIC 
                                "${PROJECT_SOURCE_DIR}/include" )
    set_target_properties( decode_tool PROPERTIES
                            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
    
    add_executable( decode_test decode_test.cpp )
    target_compile_options(decode_test PUBLIC -g -Wall)
    target_link_libraries(decode_test Decoder)
    target_include_directories( decode_test 
                                PUBLIC 
                                "${PROJECT_SOURCE_DIR}/include" )
    set_target_properties( decode_test PROPERTIES
                            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
    
    add_executable( psc_test psc_test.cpp )
    target_compile_options(psc_test PUBLIC -g -Wall)
    target_link_libraries(psc_test Decoder)
    target_include_directories( psc_test 
                                PUBLIC 
                                "${PROJECT_SOURCE_DIR}/include" )
    set_target_properties( psc_test PROPERTIES
                            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
    add_executable( trace_test trace_test.cpp )
target_compile_options( trace_test PUBLIC -g -Wall)
target_link_libraries(trace_test Tracer)
target_include_directories( trace_test 
							PUBLIC 
							"${PROJECT_SOURCE_DIR}/include" )
set_target_properties( trace_test PROPERTIES
						RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

else()
    message(FATAL_ERROR "This project is linux project")
endif()
