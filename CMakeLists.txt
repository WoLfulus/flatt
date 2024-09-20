cmake_minimum_required(VERSION 3.27)

include(cmake/env.cmake)
load_env(${CMAKE_CURRENT_SOURCE_DIR})

project(flatt CXX)

# Executable

add_executable(${PROJECT_NAME}
  src/main.cpp
)

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /bigobj)
endif()


target_precompile_headers(${PROJECT_NAME}
  PRIVATE
    "src/pch.h"
)

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

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only)

find_package(EnTT CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE EnTT::EnTT)

find_package(Lua REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${LUA_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LUA_LIBRARIES})

# Post

get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

set(TRIPLET_NAME ${VCPKG_TARGET_TRIPLET})
if(${VCPKG_HOST_TRIPLET})
  set(TRIPLET_NAME ${VCPKG_HOST_TRIPLET})
endif()

find_program(FLATC_BIN NAMES flatc PATHS "${VCPKG_INSTALLED_DIR}/${TRIPLET_NAME}/tools/flatbuffers" REQUIRED)
message(STATUS "Using flatc: ${FLATC_BIN}")


# add_custom_command(
#   TARGET ${PROJECT_NAME} PRE_BUILD
#   COMMAND
#     (${FLATC_BIN}
#       --gen-all
#       --gen-mutable
#       --gen-compare
#       --gen-object-api
#       --gen-name-strings
#       --force-empty
#       --force-empty-vectors
#       --reflect-types
#       --reflect-names
#       --cpp
#       --cpp-include array
#       --cpp-std c++17
#       --cpp-static-reflection
#       --filename-suffix ""
#       -o "${CMAKE_CURRENT_SOURCE_DIR}/src/reflection"
#       "${CMAKE_CURRENT_SOURCE_DIR}/src/reflection/reflection.fbs")
#   VERBATIM
# )


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
