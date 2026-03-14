#pragma once

namespace components
{
	class game_settings final : public common::loader::component_module
	{
	public:
		game_settings();
		~game_settings() = default;

		static inline game_settings* p_this = nullptr;
		static auto get() { return &vars; }

		static bool is_initialized()
		{
			if (p_this && p_this->m_initialized) {
				return true;
			}
			return false;
		}

		static void write_toml();
		static bool parse_toml();

		static void xo_gamesettings_update_fn();
		static void delayed_init();

		union var_value
		{
			bool boolean;
			int integer;
			float value[4] = {};
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

			const bool& _bool(const bool default_value = false) const
			{
				assert(m_type == var_type_boolean && "Type mismatch: expected boolean");
				return !default_value ? m_var.boolean : m_var_default.boolean;
			}

			const bool* _bool_ptr(const bool default_value = false)
			{
				assert(m_type == var_type_boolean && "Type mismatch: expected boolean");
				return &(!default_value ? m_var.boolean : m_var_default.boolean);
			}

			const float& _float(const bool default_value = false) const
			{
				assert(m_type == var_type_value && "Type mismatch: expected float");
				return !default_value ? m_var.value[0] : m_var_default.value[0];
			}

			const float* _float_ptr(const bool default_value = false)
			{
				assert(m_type == var_type_value && "Type mismatch: expected float");
				return !default_value ? m_var.value : m_var_default.value;
			}

			const float& _vec_x(const bool default_value = false) const
			{
				assert(((m_type == var_type_vec2) || (m_type == var_type_vec3) || (m_type == var_type_vec4)) && "Type mismatch: expected vec2, vec3 or vec4");
				return !default_value ? m_var.value[0] : m_var_default.value[0];
			}

			const float& _vec_y(const bool default_value = false) const
			{
				assert(((m_type == var_type_vec2) || (m_type == var_type_vec3) || (m_type == var_type_vec4)) && "Type mismatch: expected vec2, vec3 or vec4");
				return !default_value ? m_var.value[1] : m_var_default.value[1];
			}

			const float& _vec_z(const bool default_value = false) const
			{
				assert(((m_type == var_type_vec3) || (m_type == var_type_vec4)) && "Type mismatch: expected vec2, vec3 or vec4");
				return !default_value ? m_var.value[2] : m_var_default.value[2];
			}

			const float& _vec_w(const bool default_value = false) const
			{
				assert(m_type == var_type_vec4 && "Type mismatch: expected vec2, vec3 or vec4");
				return !default_value ? m_var.value[3] : m_var_default.value[3];
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
						assert(m_type == var_type_boolean && "Type mismatch: expected boolean");
						return &(!default_val ? m_var.boolean : m_var_default.boolean);
					}
					else if constexpr (std::is_same_v<base_type, int>) {
						assert(m_type == var_type_integer && "Type mismatch: expected integer");
						return &(!default_val ? m_var.integer : m_var_default.integer);
					}
					else if constexpr (std::is_same_v<base_type, float>) {
						if (m_type == var_type_value) {
							return &(!default_val ? m_var.value[0] : m_var_default.value[0]);
						}
						if (m_type >= var_type_vec2 && m_type <= var_type_vec4) {
							return !default_val ? m_var.value : m_var_default.value;
						}
						assert(false && "Type mismatch: expected float or vector type");
						return nullptr;
					}
					else if constexpr (std::is_same_v<base_type, Vector2D>) {
						assert(m_type == var_type_vec2 && "Type mismatch: expected vec2 for Vector");
						return reinterpret_cast<Vector2D*>(!default_val ? m_var.value : m_var_default.value);
					}
					else if constexpr (std::is_same_v<base_type, Vector>) {
						assert(m_type == var_type_vec3 && "Type mismatch: expected vec3 for Vector");
						return reinterpret_cast<Vector*>(!default_val ? m_var.value : m_var_default.value);
					}
					else if constexpr (std::is_same_v<base_type, Vector4D>) {
						assert(m_type == var_type_vec4 && "Type mismatch: expected vec4 for Vector");
						return reinterpret_cast<Vector4D*>(!default_val ? m_var.value : m_var_default.value);
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
						assert(m_type == var_type_boolean && "Type mismatch: expected boolean");
						return static_cast<T>(!default_val ? m_var.boolean : m_var_default.boolean);
					}
					else if constexpr (std::is_same_v<T, int>) {
						assert(m_type == var_type_integer && "Type mismatch: expected integer");
						return static_cast<T>(!default_val ? m_var.integer : m_var_default.integer);
					}
					else if constexpr (std::is_same_v<T, float>) {
						assert(m_type == var_type_value && "Type mismatch: expected float");
						return static_cast<T>(!default_val ? m_var.value[0] : m_var_default.value[0]);
					}
					else if constexpr (std::is_same_v<T, Vector2D>) {
						assert(m_type == var_type_vec2 && "Type mismatch: expected vec2 for Vector");
						return Vector2D(!default_val ? m_var.value : m_var_default.value);
					}
					else if constexpr (std::is_same_v<T, Vector>) {
						assert(m_type == var_type_vec3 && "Type mismatch: expected vec3 for Vector");
						return Vector(!default_val ? m_var.value : m_var_default.value);
					}
					else if constexpr (std::is_same_v<T, Vector4D>) {
						assert(m_type == var_type_vec4 && "Type mismatch: expected vec4 for Vector");
						return Vector4D(!default_val ? m_var.value : m_var_default.value);
					}
					else {
						static_assert(std::is_same_v<T, void>, "Unsupported return type in get_as");
						return T{};
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

			void reset() {
				m_var = m_var_default;
			}

			const char* m_name;
			const char* m_desc;

		private:
			var_value m_var;
			var_value m_var_default;
			var_type m_type;
		};

	private:
		bool m_initialized = false;

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

			variable emancipationgrill_alpha_modulate1x =
			{
				"emancipationgrill_alpha_modulate1x",
				"Use D3DTOP_MODULATE, use ADD if false",
				false
			};

			variable emancipationgrill_alpha_modulate2x =
			{
				"emancipationgrill_alpha_modulate2x",
				"Use D3DTOP_MODULATE2X, use 1x if false",
				false
			};

			variable emancipationgrill_alpha_modulate4x =
			{
				"emancipationgrill_alpha_modulate4x",
				"Use D3DTOP_MODULATE4X, use 1x if false",
				true
			};

			variable emancipationgrill_force_emissive =
			{
				"emancipationgrill_force_emissive",
				"Automatically make grill emissive and use 'emancipationgrill_emissive_scale'",
				true
			};

			variable emancipationgrill_emissive_scale =
			{
				"emancipationgrill_emissive_scale",
				"Emissive scale when 'emancipationgrill_force_emissive' is true",
				200.0f
			};

			variable emancipationgrill_color_scalar_center =
			{
				"emancipationgrill_color_scalar_center",
				"Color scale RGBA of emancipation grills",
				0.2f, 0.4f, 0.6f, 0.06f
			};

			variable emancipationgrill_color_scalar_side_emitters =
			{
				"emancipationgrill_color_scalar_side_emitters",
				"Color scale RGBA of emancipation grills side emitters",
				1.0f, 0.85f, 0.5f, 0.5f
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

			variable vgui_progress_board_emissive_offset =
			{
				"vgui_progress_board_emissive_offset",
				"Additional emissive offset applied to progress boards.",
				3.0f
			};

			variable bik_emissive_intensity =
			{
				"bik_emissive_intensity",
				"Emissive Intensity of BIK videos",
				1.0f
			};

			variable enable_dual_layered_water =
			{
				"enable_dual_layered_water",
				"Draws a secondary water layer over water.",
				true
			};
		};

		static inline var_definitions vars = {};
	};
}
