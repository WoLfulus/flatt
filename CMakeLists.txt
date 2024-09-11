cmake_minimum_required(VERSION 3.20)

project(flatt)

include(cmake/env.cmake)
load_env()

find_package(argparse REQUIRED)
find_package(cryptopp REQUIRED)
find_package(flatbuffers REQUIRED)
find_package(inja REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(sol2 REQUIRED)
find_package(spdlog REQUIRED)
find_package(EnTT REQUIRED)

add_executable(${PROJECT_NAME} )
