cmake_minimum_required(VERSION 3.27)

include(cmake/env.cmake)
load_env(${CMAKE_CURRENT_SOURCE_DIR})

project(flatt CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Executable

add_executable(${PROJECT_NAME}
  src/main.cpp
  src/hash.cpp
  src/io.cpp
  src/strings.cpp
  src/templates.cpp
  src/modules.cpp
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
  SOL_ALL_SAFETIES_ON=1
)

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /bigobj)
endif()

# target_precompile_headers(${PROJECT_NAME}
#   PRIVATE
#     "src/pch.hpp"
# )

# Packages

find_package(argparse CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE argparse::argparse)

find_package(cryptopp CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE cryptopp::cryptopp)

find_package(flatbuffers CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE flatbuffers::flatbuffers)

find_package(inja CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE pantor::inja)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

find_package(sol2 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE sol2)

find_package(Boost CONFIG REQUIRED COMPONENTS process)
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::process)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only)

find_package(EnTT CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE EnTT::EnTT)

find_package(Lua REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${LUA_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LUA_LIBRARIES})

# Post
#set(BOOTSTRAP_CODE "")
#file(GLOB_RECURSE BOOTSTRAP_FILES "src/bootstrap/*_*.lua")
#foreach(BOOTSTRAP_FILE ${BOOTSTRAP_FILES})
#  file(READ ${BOOTSTRAP_FILE} LOADED_CODE)
#  string(APPEND BOOTSTRAP_CODE "${LOADED_CODE}\n\n")
#endforeach()
#
#file(WRITE "src/bootstrap/bootstrap.lua" "${BOOTSTRAP_CODE}")
#file(WRITE "src/bootstrap.inc" "\nR(\"\n${BOOTSTRAP_CODE}\n\n\")\n")

set(TRIPLET_NAME ${VCPKG_TARGET_TRIPLET})
if(${VCPKG_HOST_TRIPLET})
  set(TRIPLET_NAME ${VCPKG_HOST_TRIPLET})
endif()

find_program(FLATC_BIN NAMES flatc PATHS "${VCPKG_INSTALLED_DIR}/${TRIPLET_NAME}/tools/flatbuffers" REQUIRED)
message(STATUS "Using flatc: ${FLATC_BIN}")

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FLATC_BIN} ${CMAKE_INSTALL_PREFIX}
  VERBATIM
)

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/dist/${TRIPLET_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/bin/${TRIPLET_NAME}"
  VERBATIM
)
