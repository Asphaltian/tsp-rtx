#include "std_include.hpp"
#include <wincrypt.h>

std::string hash_file_sha1(const char* file_path)
{
	const auto file = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE) {
		return {};
	}

	HCRYPTPROV prov_handle = 0;
	HCRYPTHASH hash_handle = 0;

	BYTE buffer[4096];
	DWORD bytes_read = 0;

	BYTE hash[20]; // SHA-1 produces a 20-byte hash
	DWORD hash_len = sizeof(hash);

	if (!CryptAcquireContext(&prov_handle, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT) ||
		!CryptCreateHash(prov_handle, CALG_SHA1, 0, 0, &hash_handle))
	{
		CloseHandle(file);
		return {};
	}

	while (ReadFile(file, buffer, sizeof(buffer), &bytes_read, nullptr) && bytes_read > 0)
	{
		if (!CryptHashData(hash_handle, buffer, bytes_read, 0))
		{
			CryptDestroyHash(hash_handle);
			CryptReleaseContext(prov_handle, 0);
			CloseHandle(file);
			return {};
		}
	}

	std::string hash_string;
	if (CryptGetHashParam(hash_handle, HP_HASHVAL, hash, &hash_len, 0))
	{
		std::ostringstream oss;
		for (DWORD i = 0; i < hash_len; ++i) {
			oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
		}

		hash_string = oss.str();
	}

	CryptDestroyHash(hash_handle);
	CryptReleaseContext(prov_handle, 0);
	CloseHandle(file);
	return hash_string;
}

std::unordered_set<HWND> wnd_class_list; 
BOOL CALLBACK enum_windows_proc(HWND hwnd, LPARAM lParam)
{
	DWORD window_pid, target_pid = static_cast<DWORD>(lParam);
	GetWindowThreadProcessId(hwnd, &window_pid);

	if (window_pid == target_pid && IsWindowVisible(hwnd))
	{
		char class_name[256];
		GetClassNameA(hwnd, class_name, sizeof(class_name));

		if (!wnd_class_list.contains(hwnd))
		{
			char debug_msg[256];
			wsprintfA(debug_msg, "[RTX-COMP] |> HWND: %p, PID: %u, Class: %s, Visible: %d \n", hwnd, window_pid, class_name, IsWindowVisible(hwnd));
			std::cout << debug_msg;
			wnd_class_list.insert(hwnd);
		}

		if (std::string_view(class_name).contains("Valve001"s))
		{
			glob::main_window = hwnd;
			return FALSE;
		}
	}

	return TRUE;
}

void init_fail_msg_setup()
{
	Beep(300, 100); Sleep(100); Beep(200, 100);
	game::console(); std::cout << "[RTX-COMP] Not loading P2-RTX Compatibility Mod" << std::endl;
}

void init_fail_msg_post()
{
	std::cout << "\n\tMake sure that:" << std::endl;
	std::cout << "\t- Steam is running." << std::endl;
	std::cout << "\t- That it is a legit copy of the game." << std::endl;
	std::cout << "\t- That you followed the install instructions and installed everything correctly." << std::endl;
	std::cout << "\t- Please run the game with '-debug' launch argument and:" << std::endl;
	std::cout << "\n\tCopy/paste the contents of this window when you open a GitHub issue." << std::endl;
}

#define GET_MODULE_HANDLE(HANDLE_OUT, NAME, T) \
	while (!(HANDLE_OUT)) { \
		if ((HANDLE_OUT) = (DWORD)GetModuleHandleA(NAME); !(HANDLE_OUT)) { \
			Sleep(100); (T) += 100u; \
			if ((T) >= 30000) { \
				init_fail_msg_setup(); std::cout << "[RTX-COMP] ---------------> Failed to find module: " << (NAME) << std::endl; init_fail_msg_post(); \
				return TRUE; \
			} \
		} \
	}

DWORD WINAPI find_game_window_by_sha1([[maybe_unused]] LPVOID lpParam)
{
	std::uint32_t T = 0;

	char exe_path[MAX_PATH]; GetModuleFileNameA(nullptr, exe_path, MAX_PATH);
	const std::string sha1 = hash_file_sha1(exe_path);

	if (sha1 != (IS_LATEST_BUILD ? "754149fc8da2e131c2f13324c9e087f2a690f197" : "393ca001b796245e2d5425dd3505627810daecf8")) {
		std::cout << "[RTX-COMP] ---------------> Unexpected portal2.exe hash. Hash was: " << sha1.c_str() << std::endl;
	}
	else {
		std::cout << "[RTX-COMP] Correct SHA1 Hash of portal2.exe." << std::endl;
	}

	std::cout << "[RTX-COMP] Path to exe: " << exe_path << std::endl;
	std::cout << "[RTX-COMP] Waiting for window with classname containing 'Valve001' ... \n";

	{
		while (!glob::main_window)
		{
			EnumWindows(enum_windows_proc, static_cast<LPARAM>(GetCurrentProcessId()));
			if (!glob::main_window) {
				Sleep(1u); T += 1u;
			}

			if (T >= 30000)
			{
				Beep(300, 100); Sleep(100); Beep(200, 100);
				game::console(); std::cout << "[RTX-COMP] ---------------> Could not find Valve001 Window. Not loading RTX Compatibility Mod.\n";
				return TRUE;
			}
		}
	}

	GET_MODULE_HANDLE(game::shaderapidx9_module, "shaderapidx9.dll", T);
	GET_MODULE_HANDLE(game::studiorender_module, "StudioRender.dll", T);
	GET_MODULE_HANDLE(game::engine_module, "engine.dll", T);
	GET_MODULE_HANDLE(game::client_module, "client.dll", T);
	GET_MODULE_HANDLE(game::server_module, "server.dll", T);
	GET_MODULE_HANDLE(game::vstdlib_module, "vstdlib.dll", T);

	if (const auto MH_INIT_STATUS = MH_Initialize(); MH_INIT_STATUS != MH_STATUS::MH_OK)
	{
		std::cout << "[RTX-COMP] ---------------> MinHook failed to initialize with code: " << MH_INIT_STATUS << "\n";
		return TRUE;
	}

#ifdef DEBUG
	Beep(523, 100);
#else
	if (glob::has_debug_arg) {
		Beep(523, 100);
	}
#endif

#ifdef GIT_DESCRIBE
	SetWindowTextA(glob::main_window, IS_LATEST_BUILD ? utils::va("Portal 2 - RTX - %s", GIT_DESCRIBE) : utils::va("Portal 2 - RTX - %s - DEV", GIT_DESCRIBE));
#else
	SetWindowTextA(glob::main_window, "Portal 2 - RTX");
#endif

	loader::initialize();
	return TRUE;
}

BOOL APIENTRY DllMain(HMODULE, const DWORD ul_reason_for_call, LPVOID)
{
#ifdef DEBUG
	game::console();
#else
	if (glob::has_debug_arg = flags::has_flag("debug"); glob::has_debug_arg) {
		game::console();
	}
#endif

	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		CreateThread(nullptr, 0, find_game_window_by_sha1, nullptr, 0, nullptr);
	}

	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		loader::uninitialize();
	}

	return TRUE;
}
