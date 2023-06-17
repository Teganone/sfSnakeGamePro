cmake_minimum_required(VERSION 3.16)

project(sfSnakePro)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()


file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(main ${SOURCES})
target_include_directories(main PUBLIC src ${CMAKE_CURRENT_SOURCE_DIR}/3rd/include)
target_link_directories(main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/3rd/lib)
target_link_libraries(main PRIVATE sfml-system-d sfml-window-d sfml-graphics-d sfml-audio-d sfml-network-d)
set_property(TARGET main PROPERTY
  CXX_STANDARD          17
  CXX_STANDARD_REQUIRED ON)

file(COPY "assets" DESTINATION "${CMAKE_BINARY_DIR}/${NAME}")

