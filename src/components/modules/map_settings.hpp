#pragma once
#include "remix_vars.hpp"

namespace components
{
	class map_settings : public component
	{
	public:
		map_settings();
		~map_settings();

		static inline map_settings* p_this = nullptr;
		static map_settings* get() { return p_this; }

		enum TRANSITION_MODE : uint8_t
		{
			ONCE_ON_ENTER = 0,
			ONCE_ON_LEAVE = 1,
			ALWAYS_ON_ENTER = 2,
			ALWAYS_ON_LEAVE = 3,
		};

		enum TRANSITION_TRIGGER_TYPE : uint8_t
		{
			CHOREO = 0,
			SOUND = 1,
			LEAF = 2,
		};

		struct remix_transition_s
		{
			TRANSITION_TRIGGER_TYPE trigger_type;

			// choreo trigger
			std::string choreo_name;
			std::string choreo_actor;
			std::string choreo_event;
			std::string choreo_param1;

			// sound trigger
			std::uint32_t sound_hash;
			std::string sound_name;

			// leaf trigger
			std::unordered_set<std::uint32_t> leafs;

			std::string config_name;
			TRANSITION_MODE mode;
			remix_vars::EASE_TYPE interpolate_type;
			float delay_in = 0.0f;
			float delay_out = 0.0f;
			float duration = 0.0f;
			std::uint64_t hash;
			bool _state_enter = false;
		};

		struct marker_settings_s
		{
			std::uint32_t index = 0;
			Vector origin = {};
			bool no_cull = false;
			Vector rotation = { 0.0f, 0.0f, 0.0f };
			Vector scale = { 1.0f, 1.0f, 1.0f }; // no_cull only
			std::unordered_set<std::uint32_t> areas; // no_cull only
			std::unordered_set<std::uint32_t> when_not_in_leafs; // no_cull only
			std::string comment;

			void* handle = nullptr; // internal use
			bool is_hidden = false; // internal use
		};

		struct api_config_var
		{
			std::string variable;
			std::string value;
		};

		struct api_texture_category_tweak
		{
			std::unordered_set<std::string> add_hashes;
			std::unordered_set<std::string> remove_hashes;
		};

		struct remix_light_settings_s
		{
			struct point_s
			{
				Vector position;
				Vector radiance;
				float radiance_scalar = 1.0f;
				float radius = 1.0f;
				float timepoint = 0.0f;
				float smoothness = 0.5f;

				// shaping
				bool use_shaping = false;
				Vector direction = { 0.0f, 0.0f, 1.0f };
				float degrees = 180.0; // cone angle
				float softness = 0.0f; // cone
				float exponent = 0.0f; // focus

				// volumetric
				float volumetric_scale = 1.0f;
			};

			std::vector<point_s> points;
			bool run_once = false;
			bool loop = false;
			bool loop_smoothing = false;
			bool trigger_always = false;

			std::string trigger_choreo_name;
			std::string trigger_choreo_actor;
			std::string trigger_choreo_event;
			std::string trigger_choreo_param1;
			std::uint32_t trigger_sound_hash;
			float trigger_delay = 0.0f;

			std::string kill_choreo_name;
			std::uint32_t kill_sound_hash;
			float kill_delay = 0.0f;

			float attach_prop_radius = 0.0f;
			std::string attach_prop_name;
			Vector attach_prop_mins; // min bounds
			Vector attach_prop_maxs; // max bounds

			std::string comment;
		};

		// ---

		static constexpr float DEFAULT_NOCULL_DIST = 600.0f;

		static constexpr const char* AREA_CULL_MODE_STR[] =
		{
			"NoFrustum",
			"NoFrstmInAr",
			"Stock",
			"ForceAr",
			"AreaDist",
			"Distance"
		};

		enum AREA_CULL_MODE : uint8_t
		{
			AREA_CULL_MODE_NO_FRUSTUM = 0,					// no frustum culling (everywhere)
			AREA_CULL_MODE_NO_FRUSTUM_IN_CURRENT_AREA = 1,	// no frustum culling in current area
			AREA_CULL_MODE_STOCK = 2,						// OG: frustum culling
			AREA_CULL_MODE_FORCE_AREA = 3,					// frustum culling (outside current area) + force all leafs/nodes in current area
			AREA_CULL_MODE_FORCE_AREA_DISTANCE = 4,			// frustum culling (outside current area) + force all leafs/nodes in current area and outside of current area within certain dist to player
			AREA_CULL_MODE_DISTANCE = 5,					// force all leafs/nodes within certain dist to player
			// -------------------
			AREA_CULL_INFO_COUNT = 6,
			AREA_CULL_INFO_DEFAULT = AREA_CULL_MODE_DISTANCE,
			AREA_CULL_INFO_NOCULLDIST_START = AREA_CULL_MODE_FORCE_AREA_DISTANCE,
			AREA_CULL_INFO_NOCULLDIST_END = AREA_CULL_MODE_DISTANCE,
		};

		struct leaf_tweak_s
		{
			std::unordered_set<std::uint32_t> in_leafs;
			std::unordered_set<std::uint32_t> areas;
			std::unordered_set<std::uint32_t> leafs;
			float nocull_dist = 0.0f;
		};

		struct hide_area_s
		{
			std::unordered_set<std::uint32_t> areas;
			std::unordered_set<std::uint32_t> when_not_in_leafs;
		};

		struct area_overrides_s
		{
			std::unordered_set<std::uint32_t> leafs;
			std::unordered_set<std::uint32_t> areas;
			std::unordered_set<std::uint32_t> hide_leafs;
			std::vector<hide_area_s> hide_areas;
			std::vector<leaf_tweak_s> leaf_tweaks;
			AREA_CULL_MODE cull_mode;
			float nocull_distance = DEFAULT_NOCULL_DIST;
			bool nocull_distance_overrides_in_leaf_twk = false;
			std::uint32_t area_index;
		};

		struct hide_models_s
		{
			std::unordered_set<std::string> substrings;
			std::unordered_set<float> radii;
		};

		struct map_settings_s
		{
			std::string	mapname;
			float fog_dist = 0.0f;
			float fog_density = 0.0f;
			DWORD fog_color = 0xFFFFFFFF;
			float water_uv_scale = 1.0f;
			float water_uv_top_scale = 0.0f;
			float water_offset_top = 0.5f; // top layer
			float water_offset_bottom = 0.0f; // bottom layer
			std::unordered_map<std::uint32_t, area_overrides_s> area_settings;
			float default_nocull_dist = DEFAULT_NOCULL_DIST;
			hide_models_s hide_models;
			std::unordered_set<std::string> unbake_models;
			std::vector<remix_transition_s> remix_transitions;
			std::vector<marker_settings_s> map_markers;
			std::vector<std::string> api_var_configs;
			std::unordered_map<std::string, api_texture_category_tweak> api_texture_category_tweaks;
			std::vector<remix_light_settings_s> remix_lights;
			bool using_any_light_sound_hash = false;
			bool using_any_light_attached_to_prop = false;
			bool using_any_transition_sound_hash = false;
			bool using_any_transition_sound_name = false;
		};

		static map_settings_s& get_map_settings() { return m_map_settings; }
		static const std::string& get_map_name() { return m_map_settings.mapname; }

		void set_settings_for_map(const std::string& map_name);
		static void spawn_markers_once();
		static void destroy_markers();
		static void handle_texture_category_tweaks(bool invert = false);
		static void on_map_load(const std::string& map_name);
		static void on_map_unload();
		static void clear_map_settings();
		static void reload();

		struct level_bool_s
		{
			bool sp_a1_intro1 = false,			sp_a1_intro2 = false,			sp_a1_intro3 = false,			sp_a1_intro4 = false,
				 sp_a1_intro5 = false,			sp_a1_intro6 = false,			sp_a1_intro7 = false,			sp_a1_wakeup = false,
				 sp_a2_intro = false,			sp_a2_laser_intro = false,		sp_a2_laser_stairs = false,		sp_a2_dual_lasers = false,
				 sp_a2_laser_over_goo = false,	sp_a2_catapult_intro = false,	sp_a2_trust_fling = false,		sp_a2_pit_flings = false,
				 sp_a2_fizzler_intro = false,	sp_a2_sphere_peek = false,		sp_a2_ricochet = false,			sp_a2_bridge_intro = false,
				 sp_a2_bridge_the_gap = false,	sp_a2_turret_intro = false,		sp_a2_laser_relays = false,		sp_a2_turret_blocker = false,
				 sp_a2_laser_vs_turret = false, sp_a2_pull_the_rug = false,		sp_a2_column_blocker = false,	sp_a2_laser_chaining = false,
				 sp_a2_triple_laser = false,	sp_a2_bts1 = false,				sp_a2_bts2 = false,				sp_a2_bts3 = false,
				 sp_a2_bts4 = false,			sp_a2_bts5 = false,				sp_a2_bts6 = false,				sp_a2_core = false,
				 sp_a3_00 = false,				sp_a3_01 = false,				sp_a3_03 = false,				sp_a3_jump_intro = false,
				 sp_a3_bomb_flings = false,		sp_a3_crazy_box = false,		sp_a3_transition01 = false,		sp_a3_speed_ramp = false,
				 sp_a3_speed_flings = false,	sp_a3_portal_intro = false,		sp_a3_end = false,				sp_a4_intro = false,
				 sp_a4_tb_intro = false,		sp_a4_tb_trust_drop = false,	sp_a4_tb_wall_button = false,	sp_a4_tb_polarity = false,
				 sp_a4_tb_catch = false,		sp_a4_stop_the_box = false,		sp_a4_laser_catapult = false,	sp_a4_laser_platform = false,
				 sp_a4_speed_tb_catch = false,	sp_a4_jump_polarity = false,	sp_a4_finale1 = false,			sp_a4_finale2 = false,
				 sp_a4_finale3 = false,			sp_a4_finale4 = false;

			void update(const std::string& n)
			{
					 if (n == "sp_a1_intro1") sp_a1_intro1 = true;
				else if (n == "sp_a1_intro2") sp_a1_intro2 = true;
				else if (n == "sp_a1_intro3") sp_a1_intro3 = true;
				else if (n == "sp_a1_intro4") sp_a1_intro4 = true;
				else if (n == "sp_a1_intro5") sp_a1_intro5 = true;
				else if (n == "sp_a1_intro6") sp_a1_intro6 = true;
				else if (n == "sp_a1_intro7") sp_a1_intro7 = true;
				else if (n == "sp_a1_wakeup") sp_a1_wakeup = true;
				else if (n == "sp_a2_intro") sp_a2_intro = true;
				else if (n == "sp_a2_laser_intro") sp_a2_laser_intro = true;
				else if (n == "sp_a2_laser_stairs") sp_a2_laser_stairs = true;
				else if (n == "sp_a2_dual_lasers") sp_a2_dual_lasers = true;
				else if (n == "sp_a2_laser_over_goo") sp_a2_laser_over_goo = true;
				else if (n == "sp_a2_catapult_intro") sp_a2_catapult_intro = true;
				else if (n == "sp_a2_trust_fling") sp_a2_trust_fling = true;
				else if (n == "sp_a2_pit_flings") sp_a2_pit_flings = true;
				else if (n == "sp_a2_fizzler_intro") sp_a2_fizzler_intro = true;
				else if (n == "sp_a2_sphere_peek") sp_a2_sphere_peek = true;
				else if (n == "sp_a2_ricochet") sp_a2_ricochet = true;
				else if (n == "sp_a2_bridge_intro") sp_a2_bridge_intro = true;
				else if (n == "sp_a2_bridge_the_gap") sp_a2_bridge_the_gap = true;
				else if (n == "sp_a2_turret_intro") sp_a2_turret_intro = true;
				else if (n == "sp_a2_laser_relays") sp_a2_laser_relays = true;
				else if (n == "sp_a2_turret_blocker") sp_a2_turret_blocker = true;
				else if (n == "sp_a2_laser_vs_turret") sp_a2_laser_vs_turret = true;
				else if (n == "sp_a2_pull_the_rug") sp_a2_pull_the_rug = true;
				else if (n == "sp_a2_column_blocker") sp_a2_column_blocker = true;
				else if (n == "sp_a2_laser_chaining") sp_a2_laser_chaining = true;
				else if (n == "sp_a2_triple_laser") sp_a2_triple_laser = true;
				else if (n == "sp_a2_bts1") sp_a2_bts1 = true;
				else if (n == "sp_a2_bts2") sp_a2_bts2 = true;
				else if (n == "sp_a2_bts3") sp_a2_bts3 = true;
				else if (n == "sp_a2_bts4") sp_a2_bts4 = true;
				else if (n == "sp_a2_bts5") sp_a2_bts5 = true;
				else if (n == "sp_a2_bts6") sp_a2_bts6 = true;
				else if (n == "sp_a2_core") sp_a2_core = true;
				else if (n == "sp_a3_00") sp_a3_00 = true;
				else if (n == "sp_a3_01") sp_a3_01 = true;
				else if (n == "sp_a3_03") sp_a3_03 = true;
				else if (n == "sp_a3_jump_intro") sp_a3_jump_intro = true;
				else if (n == "sp_a3_bomb_flings") sp_a3_bomb_flings = true;
				else if (n == "sp_a3_crazy_box") sp_a3_crazy_box = true;
				else if (n == "sp_a3_transition01") sp_a3_transition01 = true;
				else if (n == "sp_a3_speed_ramp") sp_a3_speed_ramp = true;
				else if (n == "sp_a3_speed_flings") sp_a3_speed_flings = true;
				else if (n == "sp_a3_portal_intro") sp_a3_portal_intro = true;
				else if (n == "sp_a3_end") sp_a3_end = true;
				else if (n == "sp_a4_intro") sp_a4_intro = true;
				else if (n == "sp_a4_tb_intro") sp_a4_tb_intro = true;
				else if (n == "sp_a4_tb_trust_drop") sp_a4_tb_trust_drop = true;
				else if (n == "sp_a4_tb_wall_button") sp_a4_tb_wall_button = true;
				else if (n == "sp_a4_tb_polarity") sp_a4_tb_polarity = true;
				else if (n == "sp_a4_tb_catch") sp_a4_tb_catch = true;
				else if (n == "sp_a4_stop_the_box") sp_a4_stop_the_box = true;
				else if (n == "sp_a4_laser_catapult") sp_a4_laser_catapult = true;
				else if (n == "sp_a4_laser_platform") sp_a4_laser_platform = true;
				else if (n == "sp_a4_speed_tb_catch") sp_a4_speed_tb_catch = true;
				else if (n == "sp_a4_jump_polarity") sp_a4_jump_polarity = true;
				else if (n == "sp_a4_finale1") sp_a4_finale1 = true;
				else if (n == "sp_a4_finale2") sp_a4_finale2 = true;
				else if (n == "sp_a4_finale3") sp_a4_finale3 = true;
				else if (n == "sp_a4_finale4") sp_a4_finale4 = true;
			}

			void reset()
			{
				memset(this, 0, sizeof(level_bool_s));
			}
		};

		static inline level_bool_s is_level = {};

	private:
		static inline map_settings_s m_map_settings = {};
		static inline std::vector<std::string> m_args;
		static inline bool m_spawned_markers = false;
		static inline bool m_loaded = false;

		bool parse_toml();
		bool matches_map_name();
		void open_and_set_var_config(const std::string& config, bool no_error = false, bool ignore_hashes = false, const char* custom_path = nullptr);
	};
}
