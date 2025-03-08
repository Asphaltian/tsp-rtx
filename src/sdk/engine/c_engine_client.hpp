#pragma once
#define ENGINE_INTERFACE_VERSION "VEngineClient015"

namespace sdk
{
	class engine_client {
	public:
		void get_screen_size(int& width, int& height);
		components::VMatrix& world_to_screen_matrix();
		void execute_client_cmd_unrestricted(const char* m_cmd);
	};
}

