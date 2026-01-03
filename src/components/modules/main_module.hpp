#pragma once
#include "map_settings.hpp"

namespace components
{
	extern int g_current_leaf;
	extern int g_current_area;
	extern int g_current_area_all_views;
	extern Vector g_player_view_org;
	extern int g_is_rendering_our_3rd_person_body_mesh;
	extern int g_is_rendering_our_3rd_person_weapon_mesh;
	extern map_settings::area_overrides_s* g_player_current_area_override;

	namespace cmd
	{
		extern bool debug_node_vis;
	}

	class main_module final : public common::loader::component_module
	{
	public:
		main_module();

		static inline main_module* p_this = nullptr;
		static main_module* get() { return p_this; }

		static bool is_initialized()
		{
			if (p_this && p_this->m_initialized) {
				return true;
			}
			return false;
		}

		static inline std::uint64_t framecount = 0u;
		static inline LPD3DXFONT d3d_font = nullptr;

		static void force_cvars();
		static void cross_handle_map_and_game_settings();

		static void trigger_vis_logic();
		static void hud_draw_area_info();

		int  m_hud_debug_node_vis_pos[2] = { 250, 135 };
		bool m_hud_debug_node_vis_has_forced_leafs = false;
		bool m_hud_debug_node_vis_has_forced_arealeafs = false;

	private:
		bool m_initialized = false;
	};
}
