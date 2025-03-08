#pragma once

namespace sdk
{
#define VGUI_MAT_SURFACE_INTERFACE_VERSION "VGUI_Surface031"

	class surface {
	public:
		bool is_cursor_visible();
		void unlock_cursor();
		void lock_cursor();
	};
}
