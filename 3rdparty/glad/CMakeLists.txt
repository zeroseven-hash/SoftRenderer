cmake_minimum_required(VERSION 3.0.0)
project(GLAD)


add_library(GLAD STATIC include/glad/glad.h include/KHR/khrplatform.h src/glad.c)