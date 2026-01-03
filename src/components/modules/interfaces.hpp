#pragma once

namespace components
{
	class interfaces final : public common::loader::component_module
	{
	public:
		interfaces();

		static inline interfaces* p_this = nullptr;
		static interfaces* get() { return p_this; }

		static bool is_initialized()
		{
			if (p_this && p_this->m_initialized) {
				return true;
			}
			return false;
		}

		//sdk::base_client* m_client = nullptr;
		sdk::engine_client* m_engine = nullptr;
		sdk::entity_list* m_entity_list = nullptr;
		sdk::surface* m_surface = nullptr;
		sdk::c_input_stacksystem* m_input_system = nullptr;
		//sdk::player_info_manager* m_player_manager = nullptr;
		//CGlobalVarsBase* m_globals = nullptr;

private:
	bool m_initialized = false;

		template <typename m_interface>
		static m_interface* get_interface(const std::string& module_name, const std::string& interface_name);
	};
}