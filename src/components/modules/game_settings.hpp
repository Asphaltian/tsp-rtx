#pragma once

namespace components
{
	class game_settings : public component
	{
	public:
		game_settings();
		~game_settings() = default;

		static inline game_settings* p_this = nullptr;
		static auto get() { return &vars; }

		static void write_toml();
		static bool parse_toml();

		static void xo_gamesettings_update_fn();

	private:
		union var_value
		{
			bool boolean;
			int integer;
			float value[3] = {};
		};

		enum var_type : std::uint8_t
		{
			var_type_boolean = 0,
			var_type_integer = 1,
			var_type_value = 2,
			var_type_vec2 = 3,
			var_type_vec3 = 4,
			var_type_vec4 = 5,
		};

		class variable
		{
		public:
			// bool
			variable(const char* name, const char* desc, const bool boolean) :
				m_name(name), m_desc(desc), m_type(var_type_boolean)
			{
				m_var.boolean = boolean;
				m_var_default.boolean = boolean;
			}

			// int
			variable(const char* name, const char* desc, const int integer) :
				m_name(name), m_desc(desc), m_type(var_type_integer)
			{
				m_var.integer = integer;
				m_var_default.integer = integer;
			}

			// float
			variable(const char* name, const char* desc, const float value) :
				m_name(name), m_desc(desc), m_type(var_type_value)
			{
				m_var.value[0] = value;
				m_var_default.value[0] = value;
			}

			// vec2
			variable(const char* name, const char* desc, const float x, const float y) :
				m_name(name), m_desc(desc), m_type(var_type_vec2)
			{
				m_var.value[0] = x; m_var.value[1] = y;
				m_var_default.value[0] = x; m_var_default.value[1] = y;
			}

			// vec3
			variable(const char* name, const char* desc, const float x, const float y, const float z) :
				m_name(name), m_desc(desc), m_type(var_type_vec3)
			{
				m_var.value[0] = x; m_var.value[1] = y; m_var.value[2] = z;
				m_var_default.value[0] = x; m_var_default.value[1] = y; m_var_default.value[2] = z;
			}

			// vec4
			variable(const char* name, const char* desc, const float x, const float y, const float z, const float w) :
				m_name(name), m_desc(desc), m_type(var_type_vec4)
			{
				m_var.value[0] = x; m_var.value[1] = y; m_var.value[2] = z; m_var.value[3] = w;
				m_var_default.value[0] = x; m_var_default.value[1] = y; m_var_default.value[2] = z; m_var_default.value[3] = w;
			}

			const char* get_str_value(bool get_default = false) const
			{
				const auto pvec = !get_default ? &m_var.value[0] : &m_var_default.value[0];

				switch (m_type)
				{
				case var_type_boolean:
					return utils::va("%s", (!get_default ? m_var.boolean : m_var_default.boolean) ? "true" : "false");

				case var_type_integer:
					return utils::va("%d", !get_default ? m_var.integer : m_var_default.integer);

				case var_type_value:
					return utils::va("%.2f", pvec[0]);

				case var_type_vec2:
					return utils::va("[ %.2f, %.2f ]", pvec[0], pvec[1]);

				case var_type_vec3:
					return utils::va("[ %.2f, %.2f, %.2f ]", pvec[0], pvec[1], pvec[2]);

				case var_type_vec4:
					return utils::va("[ %.2f, %.2f, %.2f, %.2f ]", pvec[0], pvec[1], pvec[2], pvec[3]);

				}

				return nullptr;
			}

			const char* get_str_type() const
			{
				switch (m_type)
				{
				case var_type_boolean:
					return "BOOL";

				case var_type_integer:
					return "INT";

				case var_type_value:
					return "FLOAT";

				case var_type_vec2:
					return "VEC2";

				case var_type_vec3:
					return "VEC3";

				case var_type_vec4:
					return "VEC4";
				}

				return nullptr;
			}

			std::string get_tooltip_string() const
			{
				std::string out;
				out += "# " + std::string(this->m_desc) + "\n";
				out += "# Type: " + std::string(this->get_str_type()) + " || Default: " + std::string(this->get_str_value(true));
				return out;
			}

			template <typename T>
			T get_as(bool default_val = false)
			{
				// if T is a pointer type, return a ptr
				if constexpr (std::is_pointer_v<T>)
				{
					// get the underlying type (e.g., int from int*)
					using base_type = std::remove_pointer_t<T>;

					if constexpr (std::is_same_v<base_type, bool>) {
						return &(!default_val ? m_var.boolean : m_var_default.boolean);
					}

					else if constexpr (std::is_same_v<base_type, int>) {
						return &(!default_val ? m_var.integer : m_var_default.integer);
					}

					else if constexpr (std::is_same_v<base_type, float>) {
						return &(!default_val ? m_var.value[0] : m_var_default.value[0]);
					}

					// vec2, vec3, vec4 
					else if constexpr (std::is_same_v<base_type, float[4]>) {
						return !default_val ? m_var.value : m_var_default.value;
					}

					else {
						static_assert(std::is_same_v<T, void>, "Unsupported pointer type in get_as");
						return nullptr;
					}
				}

				// return by value for non-pointer types
				else
				{
					if constexpr (std::is_same_v<T, bool>) {
						return static_cast<T>(!default_val ? m_var.boolean : m_var_default.boolean);
					}

					else if constexpr (std::is_same_v<T, int>) {
						return static_cast<T>(!default_val ? m_var.integer : m_var_default.integer);
					}

					else if constexpr (std::is_same_v<T, float>) {
						return static_cast<T>(!default_val ? m_var.value[0] : m_var_default.value[0]);
					}

					else {
						static_assert(std::is_same_v<T, void>, "Unsupported return type in get_as");
						return 0;
					}
				}
			}

			var_type get_type() const {
				return m_type;
			}

			// sets var and writes toml (bool)
			void set_var(const bool boolean, bool no_toml_update = false)
			{
				m_var.boolean = boolean;
				if (!no_toml_update) {
					write_toml();
				}
			}

			// sets var and writes toml (integer)
			void set_var(const int integer, bool no_toml_update = false)
			{
				m_var.integer = integer;
				if (!no_toml_update) {
					write_toml();
				}
			}

			// sets var and writes toml (float)
			void set_var(const float value, bool no_toml_update = false)
			{
				m_var.value[0] = value;
				if (!no_toml_update) {
					write_toml();
				}
			}

			// sets var and writes toml (vec4)
			void set_vec(const float* v, bool no_toml_update = false)
			{
				switch (m_type)
				{
				default:
					break;

				case var_type_value:
					m_var.value[0] = v[0];
					break;

				case var_type_vec2:
					m_var.value[0] = v[0]; m_var.value[1] = v[1];
					break;

				case var_type_vec3:
					m_var.value[0] = v[0]; m_var.value[1] = v[1]; m_var.value[2] = v[2];
					break;

				case var_type_vec4:
					m_var.value[0] = v[0]; m_var.value[1] = v[1]; m_var.value[2] = v[2]; m_var.value[3] = v[3];
					break;
				}

				if (!no_toml_update) {
					write_toml();
				}
			}

			const char* m_name;
			const char* m_desc;

		private:
			var_value m_var;
			var_value m_var_default;
			var_type m_type;
		};

		struct var_definitions
		{
			variable lod_forcing =
			{
				"lod_forcing",
				"The mod normally forces LOD0 for everything. Setting this to false disables that.",
				true
			};

			variable force_graphic_settings =
			{
				"force_graphic_settings",
				"This forces required graphic settings (Shader/Effect etc.)",
				true
			};

			variable default_nocull_distance =
			{
				"default_nocull_distance",
				("The default distance (radius around player) where nothing will get culled.\n"
				 "# Value is only used by certain anti-culling modes & if there isn't a manual area/leaf override via a MapSettings entry."),
				1500.00f
			};

			variable portal_visibility_culling =
			{
				"portal_visibility_culling",
				"Enable culling of exit portal areas when the entry portal is not visible from the players POV (++ performance)",
				true
			};

			variable check_nodes_for_potential_lights =
			{
				"check_nodes_for_potential_lights",
				"Check each node for potential emissive light strips (rectangular with limited depth) & force-draw node on match.",
				true
			};

			variable spotlight_billboard_spawning =
			{
				"spotlight_billboard_spawning",
				"Spawn billboard sprites (fake volumetrics) on light props",
				false
			};

			variable emancipationgrill_emissive_proxy_old =
			{
				"emancipationgrill_emissive_proxy_old",
				"Spawns an additional surface on emancipation grills that can be turned into an invisible, but emissive surface using the toolkit",
				false
			};

			variable use_brushfastpath =
			{
				"use_brushfastpath",
				("Enabling this sets cl_brushfastpath to true. The game will draw brushmodels in batches which might increase performance but reduce remix-ablitity?\n"
				"# It is forced on a3 crazy box to fix a game breaking issue with a invisible un-gel-able surface but is not tested elsewhere."),
				false
			};

			variable use_hardcoded_wheatly_flashlight_bts3 =
			{
				"use_hardcoded_wheatly_flashlight_bts3",
				"Spawns two hardcoded remixApi lights on wheatly on a2_bts3 when he turns on his flashlight. Deprecated, map_settings system now handles that.",
				false
			};

			variable debug_info_distance =
			{
				"debug_info_distance",
				"The distance cutoff (in units) were debug info such as static prop info, unbake info, bone info etc. no longer gets drawn at.",
				400.0f
			};

			variable player_backwards_offset =
			{
				"player_backwards_offset",
				"Can be used to offset the shadow casting first person player body backwards. Same logic as found within remix but without the body mesh getting smeary.",
				18.0f
			};
		};

		static inline var_definitions vars = {};
	};
}
