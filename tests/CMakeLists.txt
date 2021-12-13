cmake_minimum_required(VERSION 3.20)

project(fjson_test)


file(GLOB PUBLIC_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB fjson_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/sources DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/.)


add_executable(fjson_test ${fjson_sources})

target_link_libraries(fjson_test PUBLIC fjson_static)
