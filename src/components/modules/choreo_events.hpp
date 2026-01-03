#pragma once

namespace components
{
	namespace cmd
	{
		extern bool scene_print;
	}

	class choreo_events final : public common::loader::component_module
	{
	public:
		choreo_events();

		static inline choreo_events* p_this = nullptr;
		static choreo_events* get() { return p_this; }

		static bool is_initialized()
		{
			if (p_this && p_this->m_initialized) {
				return true;
			}
			return false;
		}

		static void on_client_frame();

	private:
			bool m_initialized = false;

	public:

		// ---
		// ---

		class event_single
		{
		public:
			void trigger()
			{
				m_triggered = true;
				m_time_point = std::chrono::steady_clock::now();
			}

			/**
			 * Can be used to check if a specified time frame has passed since the event was triggered
			 * @param seconds	Amount of seconds - Use 0 to only check if the event was triggered
			 * @return			False if event was not triggered or elapsed time smaller then specified amount of seconds
			 */
			bool has_elapsed(const float seconds = 0.0f) const
			{
				if (!m_triggered) {
					return false;
				}

				if (utils::float_equal(seconds, 0.0f)) {
					return m_triggered;
				}

				const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_time_point).count();
				return static_cast<float>(elapsed) > seconds;
			}

			void reset() {
				m_triggered = false;
			}

		private:
			bool m_triggered = false;
			std::chrono::steady_clock::time_point m_time_point;
		};

		// resets all choreo events
		static void reset_all()
		{
			ev_a4_f2_api_portal_spawn.reset();
		}

		static inline event_single ev_a4_f2_api_portal_spawn = {};

		static inline std::vector<event_single> events = {};
	};
}
