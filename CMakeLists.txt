cmake_minimum_required(VERSION 3.20)

add_compile_options(-g -Wall)

project(fjson)


file(GLOB PUBLIC_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB fjson_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)


add_executable(fjson_e ${fjson_sources})



list(FILTER fjson_sources EXCLUDE REGEX "main.c")

add_library(fjson SHARED ${fjson_sources})
add_library(fjson_static STATIC ${fjson_sources})

target_compile_options(fjson PUBLIC -fPIC)
target_compile_options(fjson_static PUBLIC -fPIC)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(
  fjson
    PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      ${CMAKE_CURRENT_BINARY_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include)


target_include_directories(
  fjson_static
    PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      ${CMAKE_CURRENT_BINARY_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include)


target_include_directories(
  fjson_e
    PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      ${CMAKE_CURRENT_BINARY_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include)


include(FetchContent)

FetchContent_Declare(
  hashmap_static
  GIT_REPOSITORY https://github.com/sebbekarlsson/hashmap.git
  )

if (NOT TARGET hashmap_static)
  FetchContent_MakeAvailable(hashmap_static)
endif()


target_link_libraries(fjson_e PUBLIC hashmap_static)
target_link_libraries(fjson PUBLIC hashmap_static)
target_link_libraries(fjson_static PUBLIC hashmap_static)
