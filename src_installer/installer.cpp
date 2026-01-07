#define MINIZ_HEADER_FILE_ONLY
#include "miniz.h"

#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <urlmon.h>
#include <objbase.h>
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>

#pragma comment(lib, "urlmon.lib")

std::string open_file_dialog()
{
	char filename[MAX_PATH] = { 0 };

	OPENFILENAMEA ofn = { 0 };
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFilter = "Portal2 Executable\0portal2.exe\0All Files\0*.*\0";
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = "Select your portal2.exe";
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrInitialDir = nullptr;

	if (GetOpenFileNameA(&ofn)) {
		return filename;
	}
		

	return "";
}

bool file_exists(const std::string& path)
{
    return GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

std::filesystem::path get_installer_dir()
{
	wchar_t buf[MAX_PATH] = { 0 };
	GetModuleFileNameW(nullptr, buf, MAX_PATH);
	return std::filesystem::path(buf).parent_path();
}

bool init_zip_from_path(const std::filesystem::path& zip_path, mz_zip_archive& zip, std::vector<char>& buffer)
{
	std::ifstream file(zip_path, std::ios::binary);
	if (!file.is_open()) {
		return false;
	}

	file.seekg(0, std::ios::end);
	const std::streamoff size = file.tellg();
	if (size <= 0) {
		return false;
	}
	buffer.resize(static_cast<size_t>(size));
	file.seekg(0, std::ios::beg);
	file.read(buffer.data(), size);
	if (!file) {
		return false;
	}

	return mz_zip_reader_init_mem(&zip, buffer.data(), buffer.size(), 0) == MZ_TRUE;
}

std::string read_file_from_zip(const std::filesystem::path& zip_path, const std::string& file_path_in_zip)
{
	std::vector<char> zip_buffer;
	mz_zip_archive zip = {};
	if (!init_zip_from_path(zip_path, zip, zip_buffer)) {
		return "";
	}

	size_t file_size = 0;
	void* file_data = mz_zip_reader_extract_file_to_heap(&zip, file_path_in_zip.c_str(), &file_size, 0);
	
	std::string result;
	if (file_data && file_size > 0) {
		result.assign(static_cast<const char*>(file_data), file_size);
		mz_free(file_data);
	}

	mz_zip_reader_end(&zip);
	return result;
}

std::string read_file_from_disk(const std::string& file_path)
{
	std::ifstream file(file_path);
	if (!file.is_open()) {
		return "";
	}
	
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return content;
}

bool extract_single_file_from_zip(const std::filesystem::path& zip_path, const std::string& file_path_in_zip, const std::filesystem::path& target_path)
{
	std::vector<char> zip_buffer;
	mz_zip_archive zip = {};
	if (!init_zip_from_path(zip_path, zip, zip_buffer)) {
		return false;
	}

	// Find the file in the zip
	int file_index = mz_zip_reader_locate_file(&zip, file_path_in_zip.c_str(), nullptr, 0);
	if (file_index < 0) {
		mz_zip_reader_end(&zip);
		return false;
	}

	// Extract to memory then write using wide-capable filesystem path to handle non-ASCII
	size_t file_size = 0;
	void* file_data = mz_zip_reader_extract_to_heap(&zip, file_index, &file_size, 0);
	bool result = false;
	if (file_data && file_size > 0) {
		std::ofstream out(target_path, std::ios::binary);
		if (out.is_open()) {
			out.write(static_cast<const char*>(file_data), file_size);
			result = static_cast<bool>(out);
		}
		mz_free(file_data);
	}

	mz_zip_reader_end(&zip);
	return result;
}

// Progress callback for URLDownloadToFileW (native Windows, no external deps)
struct DownloadProgressCallback final : IBindStatusCallback
{
	ULONG refCount = 1;
	ULONGLONG startTick = 0;
	ULONG lastProgressPercent = 101;

	// IUnknown
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
	{
		if (!ppvObject) return E_POINTER;
		*ppvObject = nullptr;
		if (riid == IID_IUnknown || riid == IID_IBindStatusCallback) {
			*ppvObject = static_cast<IBindStatusCallback*>(this);
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE AddRef(void) override
	{
		return ++refCount;
	}

	ULONG STDMETHODCALLTYPE Release(void) override
	{
		ULONG r = --refCount;
		if (r == 0) {
			delete this;
		}
		return r;
	}

	// IBindStatusCallback
	HRESULT STDMETHODCALLTYPE OnStartBinding(DWORD /*dwReserved*/, IBinding* /*pib*/) override
	{
		startTick = GetTickCount64();
		lastProgressPercent = 101;
		std::cout << "Downloading... 0%" << std::flush;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetPriority(LONG* /*pnPriority*/) override { return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE OnLowResource(DWORD /*reserved*/) override { return S_OK; }

	HRESULT STDMETHODCALLTYPE OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG /*ulStatusCode*/, LPCWSTR /*szStatusText*/) override
	{
		if (ulProgressMax == 0) {
			return S_OK;
		}

		const ULONG percent = static_cast<ULONG>((static_cast<unsigned long long>(ulProgress) * 100ull) / ulProgressMax);
		if (percent == lastProgressPercent) {
			return S_OK;
		}
		lastProgressPercent = percent;

		const ULONGLONG now = GetTickCount64();
		const double seconds = (now > startTick) ? (static_cast<double>(now - startTick) / 1000.0) : 0.0;
		const double bytes = static_cast<double>(ulProgress);
		const double bytes_per_sec = (seconds > 0.0) ? (bytes / seconds) : 0.0;
		const double mb_per_sec = bytes_per_sec / (1024.0 * 1024.0);

		std::cout << "\rDownloading... " << percent << "%";
		if (mb_per_sec > 0.01) {
			std::cout << " (" << mb_per_sec << " MB/s)";
		}
		std::cout << std::flush;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE OnStopBinding(HRESULT /*hresult*/, LPCWSTR /*szError*/) override
	{
		std::cout << "\rDownloading... done.           \n" << std::flush;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetBindInfo(DWORD* grfBINDF, BINDINFO* pbindinfo) override
	{
		if (!grfBINDF || !pbindinfo) {
			return E_POINTER;
		}

		// Use conservative defaults; URLDownloadToFileW handles the details.
		*grfBINDF = BINDF_GETNEWESTVERSION;
		ZeroMemory(pbindinfo, sizeof(BINDINFO));
		pbindinfo->cbSize = sizeof(BINDINFO);
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE OnObjectAvailable(REFIID /*riid*/, IUnknown* /*punk*/) override { return S_OK; }
	HRESULT STDMETHODCALLTYPE OnDataAvailable(DWORD /*grfBSCF*/, DWORD /*dwSize*/, FORMATETC* /*pformatetc*/, STGMEDIUM* /*pstgmed*/) override { return S_OK; }
};

bool download_file_to_path(const std::wstring& url, const std::filesystem::path& target_path)
{
	// Ensure parent directory exists
	try {
		std::filesystem::create_directories(target_path.parent_path());
	} catch (...) {
		return false;
	}

	// Ensure COM is initialized for urlmon callbacks
	HRESULT hrCo = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	DownloadProgressCallback* cb = new DownloadProgressCallback();
	HRESULT hr = URLDownloadToFileW(nullptr, url.c_str(), target_path.wstring().c_str(), 0, cb);
	cb->Release();

	if (SUCCEEDED(hrCo)) {
		CoUninitialize();
	}

	return SUCCEEDED(hr);
}

bool extract_zip(const std::filesystem::path& zip_path, const std::string& target_dir, const std::string& inner_folder = "", size_t* out_extracted_files = nullptr)
{
	std::vector<char> zip_buffer;
	mz_zip_archive zip = {};
	if (!init_zip_from_path(zip_path, zip, zip_buffer)) {
		return false;
	}

	bool result = true;
	size_t extracted_files = 0;
	mz_uint file_count = mz_zip_reader_get_num_files(&zip);
	
	// Validate output root once (canonical requires the directory to exist)
	std::filesystem::path canonical_target;
	try {
		canonical_target = std::filesystem::canonical(std::filesystem::path(target_dir));
	} catch (...) {
		mz_zip_reader_end(&zip);
		return false;
	}

	for (mz_uint i = 0u; i < file_count; i++)
	{
		mz_zip_archive_file_stat stat;
		if (!mz_zip_reader_file_stat(&zip, i, &stat)) {
			continue;
		}

		auto entry_path = std::filesystem::path(stat.m_filename);
		if (!inner_folder.empty())
		{
			std::filesystem::path inner(inner_folder);
			if (!entry_path.native().starts_with(inner.native())) {
				continue;
			}
			entry_path = entry_path.lexically_relative(inner);
		}

		std::filesystem::path out_path = std::filesystem::path(target_dir) / entry_path;
		
		// Validate output path to prevent directory traversal
		std::filesystem::path canonical_output = std::filesystem::absolute(out_path);
		if (!canonical_output.native().starts_with(canonical_target.native())) {
			continue; // Skip paths outside target directory
		}

		// Some zips (notably Windows-created zips) store directory entries with a trailing '\' instead of '/',
		// and miniz doesn't always flag them as directories. Treat trailing slash/backslash as directory.
		bool is_dir = stat.m_is_directory != 0;
		if (!is_dir && stat.m_filename) {
			const std::string name(stat.m_filename);
			if (!name.empty() && (name.back() == '/' || name.back() == '\\')) {
				is_dir = true;
			}
		}
		// Ask miniz too (handles some edge-cases)
		if (!is_dir && mz_zip_reader_is_file_a_directory(&zip, i)) {
			is_dir = true;
		}

		if (is_dir) {
			try {
				create_directories(out_path);
			} catch (const std::exception&) {
				MessageBoxA(nullptr, ("Failed to create directory: " + out_path.string()).c_str(), "Error", MB_ICONERROR);
				result = false;
			}
			continue;
		}

		try {
			create_directories(out_path.parent_path());
		} catch (const std::exception&) {
			MessageBoxA(nullptr, ("Failed to create directory: " + out_path.parent_path().string()).c_str(), "Error", MB_ICONERROR);
			result = false;
			continue;
		}

		if (i > 0 && (i % 30 == 0)) {
			Sleep(10);
		}

		if (!mz_zip_reader_extract_to_file(&zip, i, out_path.string().c_str(), 0))
		{
			MessageBoxA(nullptr, ("Failed to extract: " + std::string(stat.m_filename)).c_str(), "Error", MB_ICONERROR);
			result = false;
		}
		else
		{
			extracted_files++;
		}
	}

	mz_zip_reader_end(&zip);
	if (out_extracted_files) {
		*out_extracted_files = extracted_files;
	}
	return result;
}

int main()
{
	Sleep(200);
	
	std::cout << "Select the Portal2 directory by selecting your portal2.exe ...\n";
	Sleep(500);

	// select portal2.exe
    std::string portal2_exe_path = open_file_dialog();
	if (portal2_exe_path.empty()) 
	{
		std::cout << "Path invalid. Exiting ...\n";
		return 0;
	}

    const std::string game_dir = std::filesystem::path(portal2_exe_path).parent_path().string();
	
	// Validate game directory exists
	if (!std::filesystem::exists(game_dir) || !std::filesystem::is_directory(game_dir)) {
		MessageBoxA(nullptr, "Invalid game directory selected.", "Error", MB_ICONERROR);
		return 1;
	}
	
	std::cout << "Using Path: '" << game_dir << "'\n\n";
	
	// Find zip file first (needed for version comparison)
	static const wchar_t* zip_prefix = L"Portal2-Remix-CompatibilityMod";
	std::filesystem::path found_zip;

	for (const auto& entry : std::filesystem::directory_iterator(get_installer_dir()))
	{
		if (!entry.is_regular_file()) {
			continue;
		}

		const auto& p = entry.path();

		if (p.extension() == L".zip" && p.stem().wstring().starts_with(zip_prefix))
		{
			found_zip = p;
			break;  // take the first match
		}
	}

	if (found_zip.empty()) 
	{
		std::cout << "[ERR] Could not find any zip starting with 'Portal2-Remix-CompatibilityMod'.\n";
		MessageBoxA(nullptr, "Could not find 'Portal2-Remix-CompatibilityMod.zip' in the installer directory.", "Error", MB_ICONERROR);
		return 1;
	}
	
	// Validate zip file exists and is readable
	if (!std::filesystem::exists(found_zip) || !std::filesystem::is_regular_file(found_zip)) 
	{
		std::cout << "[ERR] Found zip file but it is not accessible: " << found_zip.string() << "\n";
		MessageBoxA(nullptr, "The zip file found is not accessible.", "Error", MB_ICONERROR);
		return 1;
	}

	Sleep(500);

	// check if comp mod and remix are installed -> update
	const bool has_remix_comp_mod = file_exists(game_dir + "\\bin\\d3d9.dll") &&
									file_exists(game_dir + "\\bin\\plugins\\p2-rtx.asi");

	if (has_remix_comp_mod) {
		std::cout << "Detected another version of the RTX Remix Compatibility Mod. Updating ... \n";
	}

	// extract comp files

	std::cout << "Extracting zip ...\n";
	Sleep(100); // Small delay before extraction

	if (!extract_zip(found_zip, game_dir, "Portal2-Remix-CompatibilityMod"))
	{
		std::cout << "[ERR] Failed to extract 'Portal2-Remix-CompatibilityMod' files from 'Portal2-Remix-CompatibilityMod.zip'\n";
		std::cout << "> Aborting installation. Please extract files manually.\n";
		return 0;
	}

	Sleep(100); // Small delay between extractions

	// Optional: download and install base remix-mod (mods folder into rtx-remix)
	{
		static const char* base_mod_zip_url = "https://github.com/xoxor4d/p2-rtx-base-mod/archive/refs/heads/master.zip";
		static const char* base_mod_repo_url = "https://github.com/xoxor4d/p2-rtx-base-mod";
		static const char* base_mod_zip_inner_mods_github = "p2-rtx-base-mod-master/mods";
		static const char* base_mod_zip_inner_mods_flat = "mods";

		// Print full info (including links) to console so the user can copy them.
		std::cout
			<< "\n\nRequired: Download and extract the base remix-mod?\n"
			<< "This contains actual remix replacements such as PBR textures, light overrides etc.\n\n"
			<< "Direct zip link: " << base_mod_zip_url << "\n"
			<< "Repo: " << base_mod_repo_url << "\n\n"
			<< "This will place the downloaded zip next to the installer, then extract the 'mods' folder into:\n"
			<< (game_dir + "\\rtx-remix\\") << "\n";

		const std::string prompt = std::string("Required: Download and extract the base remix-mod?");
		const std::filesystem::path base_zip_path = get_installer_dir() / "master.zip";
		const std::filesystem::path mods_dir = std::filesystem::path(game_dir) / "rtx-remix" / "mods";

		if (MessageBoxA(nullptr, prompt.c_str(), "Base Remix-Mod", MB_YESNO | MB_ICONQUESTION) != IDYES)
		{
			MessageBoxA(nullptr,
				("Base remix-mod is required to continue.\n\n"
				 "Please download 'master.zip' and place it next to the installer here:\n"
				 + base_zip_path.string() +
				 "\n\nLinks are printed in the console window.\n"
				 "After that, run the installer again.").c_str(),
				"Base Remix-Mod Required",
				MB_ICONERROR);
			return 0;
		}

		// If the user already downloaded it manually, reuse it.
		if (!std::filesystem::exists(base_zip_path))
		{
			std::cout << "Downloading base remix-mod zip to: " << base_zip_path.string() << "\n";
			if (!download_file_to_path(L"https://github.com/xoxor4d/p2-rtx-base-mod/archive/refs/heads/master.zip", base_zip_path))
			{
				MessageBoxA(nullptr,
					("Failed to download base remix-mod.\n\n"
					 "Please download 'master.zip' and place it next to the installer here:\n"
					 + base_zip_path.string()
					 + "\n\nLinks are printed in the console window.\n"
					 "Then run the installer again to continue.").c_str(),
					"Error",
					MB_ICONERROR);
				return 0;
			}
		}
		else
		{
			std::cout << "Found existing base remix-mod zip: " << base_zip_path.string() << "\n";
		}

		try {
			std::filesystem::create_directories(mods_dir);
		} catch (...) {
			MessageBoxA(nullptr, ("Failed to create directory: " + mods_dir.string()).c_str(), "Error", MB_ICONERROR);
			return 0;
		}

		std::cout << "Extracting base remix-mod into rtx-remix/mods ...\n";
		size_t extracted = 0;
		bool ok_extract = extract_zip(base_zip_path, mods_dir.string(), base_mod_zip_inner_mods_github, &extracted);
		if (!ok_extract || extracted == 0)
		{
			// Fallback for archives that have 'mods/...' at the root
			extracted = 0;
			ok_extract = extract_zip(base_zip_path, mods_dir.string(), base_mod_zip_inner_mods_flat, &extracted);
		}

		if (!ok_extract || extracted == 0)
		{
			MessageBoxA(nullptr,
				("Failed to extract base remix-mod.\n\nYou can extract it manually from:\n"
					+ std::string(base_mod_zip_url)
					+ "\n\nRepo:\n"
					+ std::string(base_mod_repo_url)
					+ "\n\nMake sure the extracted folder ends up here:\n"
					+ (std::filesystem::path(game_dir) / "rtx-remix" / "mods").string()).c_str(),
				"Error",
				MB_ICONERROR);
			return 0;
		}
	}

	// Only prompt about DirectX if this is a fresh install (p2-rtx.asi doesn't exist)
	if (!has_remix_comp_mod)
	{
		std::cout
			<< "\n\nIt's recommended to install Microsoft DirectX June 2010 Redistributable.\n"
			<< "https://www.microsoft.com/en-us/download/details.aspx?id=8109\n";

		// DX9 June 2010 runtime
		if (MessageBoxA(nullptr, "It's recommended to install Microsoft DirectX June 2010 Redistributable.\nDo you want to open a link to the installer?", "DirectX Runtime", MB_YESNO | MB_ICONQUESTION) == IDYES) {
			ShellExecuteA(nullptr, "open", "https://www.microsoft.com/en-us/download/details.aspx?id=8109", nullptr, nullptr, SW_SHOWNORMAL);
		}
	}

	std::cout << "\n\nIf you run into issues, please create an issue on the GitHub repository.\n> Please include 'portal2-rtx/logs/logfile.txt'\n> The log files from 'rtx-remix/logs'\n> A short description and anything else that might help to identify the issue.\n";

	MessageBoxA(nullptr, "Installation complete!\nYou can now launch Portal 2\nby running run-p2-rtx.bat", "Success", MB_ICONINFORMATION);
    return 0;
}