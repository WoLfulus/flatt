cmake_minimum_required(VERSION 3.20)

cwake_package_info(
  NAME flatt
  TYPE console
  LINK
    argparse::argparse
    cryptopp::cryptopp
    flatbuffers::flatbuffers
    nlohmann_json::nlohmann_json
    pantor::inja
    sol2::sol2
    spdlog::spdlog
    EnTT::EnTT
)

macro(on_flatt_package_setup)
  project(flatt)

  find_package(argparse REQUIRED)
  find_package(cryptopp REQUIRED)
  find_package(flatbuffers REQUIRED)
  find_package(inja REQUIRED)
  find_package(nlohmann_json REQUIRED)
  find_package(sol2 REQUIRED)
  find_package(spdlog REQUIRED)
  find_package(entt REQUIRED)
  find_package(fmt REQUIRED)
endmacro()

macro(on_flatt_package_configure)
  target_compile_options(flatt PUBLIC "/Zc:__cplusplus")
  target_compile_features(flatt PRIVATE cxx_std_23)
endmacro()
