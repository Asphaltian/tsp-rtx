#include <std_include.hpp>

namespace sdk
{
	void engine_client::get_screen_size(int& width, int& height) {
		using original_fn = void(__thiscall*)(engine_client*, int&, int&);
		return (*(original_fn * *)this)[5](this, width, height);
	}

	VMatrix& engine_client::world_to_screen_matrix()
	{
		using original_fn = VMatrix&(__thiscall*)(engine_client*);
		return (*(original_fn * *)this)[37](this);
	}
	
	void engine_client::execute_client_cmd_unrestricted(const char* m_cmd)
	{
		using original_fn = void(__thiscall*)(engine_client*, const char*);
		return (*(original_fn**)this)[108](this, m_cmd);
	}
}
