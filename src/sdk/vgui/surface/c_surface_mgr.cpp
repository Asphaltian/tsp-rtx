#include <std_include.hpp>
#include "c_surface_mgr.hpp"

namespace sdk
{
	bool surface::is_cursor_visible()
	{
		using original_fn = bool(__thiscall*)(surface*);
		return (*(original_fn**)this)[56](this);
	}

	void surface::unlock_cursor()
	{
		using original_fn = void(__thiscall*)(surface*);
		return (*(original_fn**)this)[64](this);
	}

	void surface::lock_cursor()
	{
		using original_fn = void(__thiscall*)(surface*);
		return (*(original_fn**)this)[65](this);
	}
}
