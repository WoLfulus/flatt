set shell := ["bash", "-c"]

build PLATFORM: cleanup
  ./cmake/targets/{{ PLATFORM }}/shell cmake --preset {{PLATFORM}}-debug
  ./cmake/targets/{{ PLATFORM }}/shell cmake --build --preset {{PLATFORM}}-debug

  ./cmake/targets/{{ PLATFORM }}/shell cmake --preset {{PLATFORM}}-release
  ./cmake/targets/{{ PLATFORM }}/shell cmake --build --preset {{PLATFORM}}-release


build-test PLATFORM CONFIG: cleanup
  rm -rf ./build/
  ./cmake/targets/{{ PLATFORM }}/shell conan config install ./cmake/conan/
  ./cmake/targets/{{ PLATFORM }}/shell cmake --preset {{PLATFORM}}-{{CONFIG}}
  ./cmake/targets/{{ PLATFORM }}/shell cmake --build --preset {{PLATFORM}}-{{CONFIG}}


shell PLATFORM:
  ./cmake/targets/{{ PLATFORM }}/shell bash

cleanup:
  if test -e ./CMakeUserPresets.json; then rm ./CMakeUserPresets.json; fi
