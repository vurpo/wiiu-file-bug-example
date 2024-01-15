cmake_minimum_required(VERSION 3.5)
project(file_bug_example C CXX)

add_executable(file_bug_example
   main.cpp
   )

wut_create_rpx(file_bug_example)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/file_bug_example.rpx"
        DESTINATION "${CMAKE_INSTALL_PREFIX}")
install(DIRECTORY "${PROJECT_SOURCE_DIR}/shaders"
         DESTINATION "${CMAKE_INSTALL_PREFIX}")