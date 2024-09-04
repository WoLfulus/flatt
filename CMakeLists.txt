cmake_minimum_required(VERSION 3.20)

project(flatt)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CONAN_LINK_RUNTIME "MT")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.env")
  file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/.env" entries)
  foreach (entry IN LISTS entries)
    if(entry MATCHES "^([^=]+)=(.*)$")
      set(ENV{${CMAKE_MATCH_1}} "${CMAKE_MATCH_2}")
    else()
      message(FATAL_ERROR "Malformed dotenv entry:\n${entry}")
    endif()
  endforeach()
endif()

include("./cmake/cwake.cmake")

list(APPEND
  CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
)

cwake_bootstrap(
  ${CMAKE_CURRENT_SOURCE_DIR}
)
