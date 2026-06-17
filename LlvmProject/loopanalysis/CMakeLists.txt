cmake_minimum_required(VERSION 3.20)
project(MinPass)

if(NOT DEFINED LLVM_DIR AND DEFINED ENV{LLVM_PROJECT_INSTALL})
  set(LLVM_DIR "$ENV{LLVM_PROJECT_INSTALL}/lib/cmake/llvm")
endif()

find_package(LLVM REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH ${LLVM_CMAKE_DIR})
include(AddLLVM)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

add_llvm_pass_plugin(MinPass
  pass.cpp
)
