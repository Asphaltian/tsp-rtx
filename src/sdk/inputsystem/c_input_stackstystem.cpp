#include <std_include.hpp>
#include "c_input_stackstystem.hpp"

namespace sdk
{
	//InputContext_t* c_input_stacksystem::push_input_context()
	//{
	//	return utils::hook::call_virtual<8, InputContext_t*>(interfaces::get()->m_input_system);
	//	//using original_fn = InputContext_t*(__thiscall*)(c_input_stacksystem*);
	//	//return (*(original_fn**)this)[8](this);
	//}

	//void c_input_stacksystem::enable_input_context(InputContext_t* context, bool enable)
	//{
	//	using original_fn = void(__thiscall*)(c_input_stacksystem*, InputContext_t*, bool);
	//	return (*(original_fn**)this)[10](this, context, enable);
	//}

	//void c_input_stacksystem::set_cursor_visible(InputContext_t* context, bool enable)
	//{
	//	using original_fn = void(__thiscall*)(c_input_stacksystem*, InputContext_t*, bool);
	//	return (*(original_fn**)this)[11](this, context, enable);
	//}

	//void c_input_stacksystem::set_mouse_capture(InputContext_t* context, bool enable)
	//{
	//	using original_fn = void(__thiscall*)(c_input_stacksystem*, InputContext_t*, bool);
	//	return (*(original_fn**)this)[13](this, context, enable);
	//}

	//void c_input_stacksystem::set_cursor_position(InputContext_t* context, int x, int y)
	//{
	//	using original_fn = void(__thiscall*)(c_input_stacksystem*, InputContext_t*, int, int);
	//	return (*(original_fn**)this)[14](this, context, x, y);
	//}

	//bool c_input_stacksystem::is_topmost_enabled_context(InputContext_t* context)
	//{
	//	using original_fn = bool*(__thiscall*)(c_input_stacksystem*, InputContext_t*);
	//	return (*(original_fn**)this)[15](this, context);
	//}
}
