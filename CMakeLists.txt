cmake_minimum_required(VERSION 3.20)
project(flatt)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CONAN_LINK_RUNTIME "MT")

include("./cmake/cwake.cmake")

cwake_bootstrap(
  ${CMAKE_CURRENT_SOURCE_DIR}
)
