cmake_minimum_required(VERSION 3.0.0)
project(Example VERSION 0.1.0)


set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include_directories("${CMAKE_CURRENT_LIST_DIR}/Extern/GLFW/include")
include_directories("${CMAKE_CURRENT_LIST_DIR}/Extern/GLAD/include")
include_directories("${CMAKE_SOURCE_DIR}/SoftRenderer")



add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Extern/GLFW)
set_property(TARGET glfw PROPERTY FOLDER "dependencies")

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Extern/GLAD)
set_property(TARGET GLAD PROPERTY FOLDER "dependencies")

add_executable(triangle triangle.cpp)
add_executable(texture texture.cpp)
target_link_libraries(triangle glfw GLAD SoftRenderer)
target_link_libraries(texture glfw GLAD SoftRenderer)



