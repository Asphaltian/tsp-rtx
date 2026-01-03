#pragma once

namespace components
{
	namespace cmd
	{
		extern bool sound_debug_printing;
	}

	class sound_events final : public common::loader::component_module
	{
	public:
		sound_events();

		static inline sound_events* p_this = nullptr;
		static auto get() { return p_this; }

		static bool is_initialized()
		{
			if (p_this && p_this->m_initialized) {
				return true;
			}
			return false;
		}

	private:
		bool m_initialized = false;
	};
}
