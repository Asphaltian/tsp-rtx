#pragma once
#include "components/modules/map_settings.hpp"

#define TOML_ERROR(TITLE, ENTRY, MSG, ...) \
	common::log("Toml", std::format("{}", toml::format_error(toml::make_error_info(#TITLE, (ENTRY), utils::va(#MSG, __VA_ARGS__)))), common::LOG_TYPE::LOG_TYPE_ERROR, true);

#define TOML_CATCH_ERROR_WHAT	{ common::log("Toml", std::format("{}", err.what()), common::LOG_TYPE::LOG_TYPE_ERROR, true); }

#define TOML_CATCH_SYNTAX_ERROR	catch (toml::syntax_error& err) TOML_CATCH_ERROR_WHAT
#define TOML_CATCH_TYPE_ERROR	catch (toml::type_error& err) TOML_CATCH_ERROR_WHAT

namespace common::toml
{
	std::string build_light_string_for_single_light(const map_settings::remix_light_settings_s& def);
	std::string build_map_marker_string_for_current_map(const std::vector<map_settings::marker_settings_s>& markers);
	std::string build_culling_overrides_string_for_current_map(const std::unordered_map<std::uint32_t, map_settings::area_overrides_s>& areas);
}
