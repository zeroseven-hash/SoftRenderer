cmake_minimum_required(VERSION 3.0.0)
project(Example VERSION 0.1.0)


set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include_directories("${CMAKE_CURRENT_LIST_DIR}/extern/GLFW/include")
include_directories("${CMAKE_CURRENT_LIST_DIR}/extern/GLAD/include")
include_directories("${CMAKE_SOURCE_DIR}/extern/ASSIMP/include")
include_directories("${CMAKE_SOURCE_DIR}/SoftRenderer")


add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/extern/GLFW)
set_property(TARGET glfw PROPERTY FOLDER "dependencies")

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/extern/GLAD)
set_property(TARGET GLAD PROPERTY FOLDER "dependencies")


# assimp
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_DRACO OFF CACHE BOOL "" FORCE)
set(ASSIMP_WERROR OFF CACHE BOOL "" FORCE)
unset(CMAKE_C_STANDARD)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/extern/ASSIMP)
set_property(TARGET GLAD PROPERTY FOLDER "dependencies")
set(CMAKE_C_STANDARD ${C_STANDARD_DEFAULT})

add_executable(triangle example1_triangle/example1_triangle.cpp)
add_executable(texture example2_texture/example2_texture.cpp)
add_executable(model example3_model/example3_model.cpp example3_model/Model.h )
target_link_libraries(triangle glfw GLAD SoftRenderer)
target_link_libraries(texture glfw GLAD SoftRenderer)
target_link_libraries(model glfw GLAD SoftRenderer assimp)



