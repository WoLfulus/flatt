#!/usr/bin/env just --justfile

[private]
default:
  @echo ''
  @echo 'Flatt'
  @echo ''
  @just --list
  @echo ''

[private]
initialize: builders

[private]
builders:
  docker build -t wolfulus/xc:linux-x64 -f ./cmake/builders/Dockerfile.linux ./cmake/builders/
  docker build -t wolfulus/xc:windows-x64 -f ./cmake/builders/Dockerfile.windows ./cmake/builders/

[private]
configure-cross platform:

[private]
build-cross platform:

[private]
configure-local platform:
  mkdir -p build/{{ platform }}/CMakeFiles
  cmake --preset "{{ platform }}"

[private]
build-local platform:
  mkdir -p build/{{ platform }}/CMakeFiles
  cmake --build --preset "{{ platform }}"

# Common

# Configuration

configure config="choose":
  @just configure-{{config}}

[private]
@configure-choose:
  target=`gum choose all debug release` && just "configure-$target"

[private]
@configure-all:
  just configure-debug
  just configure-release

# Build

build config="choose":
  @just build-{{config}}

[private]
@build-choose:
  target=`gum choose all debug release` && just "build-$target"

[private]
@build-all:
  just build-debug
  just build-release

# Linux

[linux, private]
@configure-debug:
  just configure-local linux-x64-debug

[linux, private]
@configure-release:
  just configure-local linux-x64-release

[linux, private]
@build-debug:
  just build-local linux-x64-debug

[linux, private]
@build-release:
  just build-local linux-x64-release

# Windows

[windows, private]
@configure-debug:
  just configure-local win-x64-debug

[windows, private]
@configure-release:
  just configure-local win-x64-release

[windows, private]
@build-debug:
  just build-local win-x64-debug

[windows, private]
@build-release:
  just build-local win-x64-release

[windows]
@pack-it-up:
  just configure all
  just build all
  wsl --exec bash -i -c "just configure all"
  wsl --exec bash -i -c "just build all"
