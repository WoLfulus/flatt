
#pragma once

#include <nlohmann/json.hpp>

#include <sol/state_view.hpp>
#include <sol/object.hpp>   // for object
#include <sol/optional.hpp> // for optional
#include <sol/table.hpp>    // for table
#include <sol/version.hpp>  // for SOL_IS_OFF, SOL_SAFE_NUMERICS, ...

#if SOL_IS_OFF(SOL_SAFE_NUMERICS) && SOL_IS_OFF(SOL_ALL_SAFETIES_ON)
  #error "nlohmann support for sol requires at least SOL_SAFE_NUMERICS=1"
  #include <force_compiler_to_stop_here>
#endif

namespace nlohmann {

  template <>
  struct adl_serializer<sol::object> {
    static void to_json_array(json& j, const sol::table& tbl) {
      if (j.type() != json::value_t::array) {
        j = json::array();
      }
      auto kv_args = json::object();
      for (auto& kv : tbl) {
        if (kv.first.get_type() != sol::type::number) {
          auto key = kv.first.as<std::string>();
          to_json(kv_args[key], kv.second.as<sol::object>());
          continue;
        }
        j.emplace_back(kv.second.as<sol::object>());
      }
      if (!kv_args.empty()) {
        j.emplace_back(std::move(kv_args));
      }
    }

    static void to_json_object(json& j, const sol::table& tbl) {
      if (j.type() != json::value_t::object) {
        j = json::object();
      }
      for (auto& kv : tbl) {
        auto key = kv.first.as<std::string>();
        to_json(j[key], kv.second.as<sol::object>());
      }
    }

    static void to_json(json& j, const sol::table& tbl) {
      if (tbl.pairs().begin() == tbl.pairs().end()) {
        // We don't know whether this is an empty array or an empty object,
        // but it's probably an array since this makes more sense to have.
        if (j.type() == json::value_t::null) {
          j = json::array();
        }
        return;
      }

      // Lua only accepts table keys that are integers or strings,
      // but they can be mixed; we use the first element to guess
      // whether its an array or an object.
      auto first = (*tbl.pairs().begin()).first;
      bool looks_like_array = (first.get_type() == sol::type::number);
      if (looks_like_array) {
        to_json_array(j, tbl);
      } else {
        to_json_object(j, tbl);
      }
    }

    static void to_json(json& j, const sol::object& obj) {
      switch (obj.get_type()) {
      case sol::type::table: {
        to_json(j, obj.as<sol::table>());
        break;
      }
      case sol::type::string: {
        j = obj.as<std::string>();
        break;
      }
      case sol::type::boolean: {
        j = obj.as<bool>();
        break;
      }
      case sol::type::number: {
        // If the number in Lua has any significant decimals, even if they are zero,
        // it is not an integer and this optional will be falsy.
        if (auto num = obj.as<sol::optional<int64_t>>(); num) {
          j = *num;
        } else {
          j = obj.as<double>();
        }
        break;
      }
      case sol::type::nil:
      case sol::type::none: {
        j = nullptr;
        break;
      }
      case sol::type::poly:
        j = "<poly>";
        break;
      case sol::type::function:
        j = "<function>";
        break;
      case sol::type::thread:
        j = "<thread>";
        break;
      case sol::type::userdata:
      case sol::type::lightuserdata:
        j = "<userdata>";
        break;
      }
    }

    static void from_json(const json& j, sol::object& obj) {
      auto* L = obj.lua_state();
      if (L == nullptr) {
        throw std::logic_error("can only deserialize to existing sol::object");
      }
      auto lua = sol::state_view(L);
      switch (j.type()) {
      case json::value_t::object: {
        auto tbl = lua.create_table();
        for (const auto& it : j.items()) {
          auto tmp = sol::object(L, sol::in_place, nullptr);
          from_json(it.value(), tmp);
          tbl[it.key()] = tmp;
        }
        obj = tbl;
        break;
      }
      case json::value_t::null: {
        obj = sol::object(L, sol::in_place, nullptr);
        break;
      }
      case json::value_t::array: {
        auto tbl = lua.create_table();
        for (const auto& el : j) {
          auto tmp = sol::object(L, sol::in_place, nullptr);
          from_json(el, tmp);
          tbl.add(tmp);
        }
        obj = tbl;
        break;
      }
      case json::value_t::binary: {
        obj = sol::object(L, sol::in_place, nullptr);
        break;
      }
      case json::value_t::string: {
        obj = sol::object(L, sol::in_place, j.get<std::string>());
        break;
      }
      case json::value_t::boolean: {
        obj = sol::object(L, sol::in_place, j.get<bool>());
        break;
      }
      case json::value_t::number_float: {
        obj = sol::object(L, sol::in_place, j.get<double>());
        break;
      }
      case json::value_t::number_unsigned: {
        obj = sol::object(L, sol::in_place, j.get<unsigned long>());
        break;
      }
      case json::value_t::number_integer: {
        obj = sol::object(L, sol::in_place, j.get<signed long>());
        break;
      }
      case json::value_t::discarded: {
        obj = sol::object(L, sol::in_place, nullptr);
        break;
      }
      }
    }
  };

} // namespace nlohmann

inline sol::object to_sol(sol::state_view& lua, const nlohmann::json& json) {
  auto tmp = sol::object(lua, sol::in_place, nullptr);
  nlohmann::adl_serializer<sol::object>::from_json(json, tmp);
  return tmp;
}

inline sol::object to_sol(sol::this_state& state, const nlohmann::json& json) {
  auto lua = sol::state_view(state);
  auto tmp = sol::object(lua, sol::in_place, nullptr);
  nlohmann::adl_serializer<sol::object>::from_json(json, tmp);
  return tmp;
}
