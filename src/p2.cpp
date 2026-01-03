#include "std_include.hpp"

#include "components/common/flags.hpp"
#include "components/common/remix_api.hpp"
#include "components/modules/choreo_events.hpp"
#include "components/modules/game_settings.hpp"
#include "components/modules/imgui.hpp"
#include "components/modules/interfaces.hpp"
#include "components/modules/main_module.hpp"
#include "components/modules/model_render.hpp"
#include "components/modules/remix_lights.hpp"
#include "components/modules/remix_rayportal.hpp"
#include "components/modules/sound_events.hpp"

namespace p2
{
	void main()
	{
		// init remix api
		/*common::remix_api::initialize(nullptr, nullptr, []
		{
			main_module::hud_draw_area_info();
		}, false);*/

		game_settings::delayed_init();

		common::loader::module_loader::register_module(std::make_unique<interfaces>());
		common::loader::module_loader::register_module(std::make_unique<imgui>());
		common::loader::module_loader::register_module(std::make_unique<game_settings>());
		common::loader::module_loader::register_module(std::make_unique<choreo_events>());
		common::loader::module_loader::register_module(std::make_unique<sound_events>());
		common::loader::module_loader::register_module(std::make_unique<remix_vars>());
		common::loader::module_loader::register_module(std::make_unique<remix_rayportal>());
		common::loader::module_loader::register_module(std::make_unique<remix_lights>());
		common::loader::module_loader::register_module(std::make_unique<map_settings>());
		common::loader::module_loader::register_module(std::make_unique<model_render>());
		common::loader::module_loader::register_module(std::make_unique<main_module>());

		MH_EnableHook(MH_ALL_HOOKS);
	}
}
