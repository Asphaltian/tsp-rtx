#pragma once
#include "utils/hooking.hpp"

namespace sdk
{
#define INPUT_SYSTEM_INTERFACE_VERSION "InputStackSystemVersion001"

	struct InputContext_t
	{
		int m_hCursorIcon;
		bool m_bEnabled;
		bool m_bCursorVisible;
		bool m_bMouseCaptureEnabled;
	};

	class c_input_stacksystem
	{
	public:
		InputContext_t* push_input_context() {
			return utils::mem::virtual_function<InputContext_t * (__thiscall*)(void*)>(this, 8)(this);
		}

		void enable_input_context(InputContext_t* hContext, bool bEnable) {
			utils::mem::virtual_function<void(__thiscall*)(void*, InputContext_t*, bool)>(this, 10)(this, hContext, bEnable);
		}

		void set_cursor_visible(InputContext_t* hContext, bool bEnable) {
			utils::mem::virtual_function<void(__thiscall*)(void*, InputContext_t*, bool)>(this, 11)(this, hContext, bEnable);
		}

		void set_mouse_capture(InputContext_t* hContext, bool bEnable) {
			utils::mem::virtual_function<void(__thiscall*)(void*, InputContext_t*, bool)>(this, 13)(this, hContext, bEnable);
		}

		void set_cursor_position(InputContext_t* hContext, int x, int y) {
			utils::mem::virtual_function<void(__thiscall*)(void*, InputContext_t*, int, int)>(this, 14)(this, hContext, x, y);
		}

		bool is_topmost_enabled_context(InputContext_t* hContext) {
			return utils::mem::virtual_function<bool(__thiscall*)(void*, InputContext_t*)>(this, 15)(this, hContext);
		}
	};

	class c_input_system {
	public:
		void enable_input(bool state) {
			// @xref: "55 8B EC 8A 45 08 88 81 ? ? ? ? 5D C2 04 00 55 8B EC 0F B6 81" [inputsystem.dll]
			utils::mem::virtual_function<void(__thiscall*)(void*, bool)>(this, 11)(this, state);
		}

		void reset_input_state() {
			// @xref: "56 57 68 ? ? ? ? 6A 00 " [inputsystem.dll]
			utils::mem::virtual_function<void(__thiscall*)(void*)>(this, 28)(this);
		}

		void get_raw_mouse_accumulators(int* x, int* y) {
			utils::mem::virtual_function<void(__thiscall*)(void*, int*, int*)>(this, 50)(this, x, y); // GetRawMouseAccumulators
		}
	};
}
