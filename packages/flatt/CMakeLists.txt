cmake_minimum_required(VERSION 3.20)

cwake_package_info(
  NAME flatt
  TYPE console
)

macro(on_flatt_package_setup)
  project(flatt)
endmacro()

macro(on_flatt_package_configure)
  target_compile_features(flatt PRIVATE cxx_std_23)
endmacro()
