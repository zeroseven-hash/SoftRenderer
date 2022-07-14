cmake_minimum_required(VERSION 3.0.0)
project(SoftRenderer VERSION 0.1.0)


file(GLOB_RECURSE all_files "${CMAKE_CURRENT_LIST_DIR}/src/**.h" "${CMAKE_CURRENT_LIST_DIR}/src/**.cpp" "${CMAKE_CURRENT_LIST_DIR}/src/**.inl")

include_directories("${CMAKE_CURRENT_LIST_DIR}/3rdparty/glfw/include")
include_directories("${CMAKE_CURRENT_LIST_DIR}/3rdparty/glad/include")
include_directories("${CMAKE_CURRENT_LIST_DIR}/3rdparty/assimp/include")
include_directories("${CMAKE_CURRENT_LIST_DIR}/3rdparty/stb_image")
include_directories("${CMAKE_CURRENT_LIST_DIR}/3rdparty/imgui")
include_directories("${CMAKE_CURRENT_LIST_DIR}/3rdparty/tinyddsloader")
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

add_subdirectory(./3rdparty)
set(CMAKE_CXX_STANDARD 17)

add_executable(SoftRenderer "${all_files}")
target_link_libraries(SoftRenderer glfw GLAD imgui assimp)

target_include_directories(SoftRenderer PRIVATE "./src")