cmake_minimum_required(VERSION 3.0.0)
project(SoftRenderer VERSION 0.1.0)


file (GLOB_RECURSE all_files "${CMAKE_CURRENT_LIST_DIR}/src/**.h" "${CMAKE_CURRENT_LIST_DIR}/src/**.cpp")



#organize files
foreach(FILE ${all_files})
  #convert source file to absolute
  get_filename_component(ABSOLUTE_PATH "${FILE}" ABSOLUTE)
  # Get the directory of the absolute source file
  get_filename_component(PARENT_DIR "${ABSOLUTE_PATH}" DIRECTORY)
  # Remove common directory prefix to make the group
  string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
  source_group("${GROUP}" FILES "${FILE}")
endforeach()
add_library(SoftRenderer STATIC "${all_files}")
target_include_directories(SoftRenderer PRIVATE ${CMAKE_CURRENT_LIST_DIR}/extern)
