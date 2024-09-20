#pragma once

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
  #define NOMINMAX
#endif

#ifdef _WIN32
  #include <windows.h>
#endif

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <regex>
#include <string>
#include <vector>
#include <optional>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <flatbuffers/reflection_generated.h>
#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/util.h>
#include <flatbuffers/minireflect.h>

// #include "reflection/reflection.h"

#include <inja/inja.hpp>
#include <nlohmann/json.hpp>

#include <cryptopp/sha.h>
#include <cryptopp/hex.h>

#include <sol/sol.hpp>

#include <entt/core/hashed_string.hpp>
