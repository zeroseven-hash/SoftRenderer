cmake_minimum_required(VERSION 3.0.0)
project(imgui)

file (GLOB files "./*.h" "./*.cpp")


add_library(imgui STATIC ${files} )
