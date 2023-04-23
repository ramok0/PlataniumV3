//Made by Ramok (@Ramokprout)

#ifndef __PLATANIUMV3__
#define __PLATANIUMV3__
#include <Windows.h>
#include <TlHelp32.h>
#include <memcury.hpp>
#include <iostream>
#include <filesystem>
#include <curlrip.h>

//patterns
static const char* CURL_EASY_SETOPT = "89 54 24 10 4C 89 44 24 ? 4C 89 4C 24 ? 48 83 EC 28 48 85 C9";
static const wchar_t* REQUEST_EXIT_WITH_STATUS = L"Cannot find a compatible Vulkan device or driver. Try updating your video driver to a more recent version and make" " sure your video card supports Vulkan.\n" "\n";
static const wchar_t* UNSAFE_ENVIRONNEMENT = L"UnsafeEnvironment_Title";
static const wchar_t* GET_ENGINE_VERSION = L"unreal-v%i-%s.dmp";
static const wchar_t* GENGINE_STRING = L"Invalid Engine used, must be FortEngine or FortUnrealEdEngine";
static const wchar_t* SHOULD_CHECK_PAK = L"heckpak";

//ue4 structures
struct FEngineVersionBase {
	/** Major version number. */
	uint16_t Major = 0;

	/** Minor version number. */
	uint16_t Minor = 0;

	/** Patch version number. */
	uint16_t Patch = 0;

	/** Changelist number. This is used to arbitrate when Major/Minor/Patch version numbers match. Use GetChangelist() instead of using this member directly. */
	uint32_t Changelist = 0;
};

enum class EIoErrorCode
{
	Ok,
	Unknown,
	InvalidCode,
	Cancelled,
	FileOpenFailed,
	FileNotOpen,
	ReadError,
	WriteError,
	NotFound,
	CorruptToc,
	UnknownChunkID,
	InvalidParameter,
	SignatureError,
	InvalidEncryptionKey,
	CompressionError,
};


class FIoStatus
{
	static constexpr int32_t MaxErrorMessageLength = 128;
	using FErrorMessage = TCHAR[MaxErrorMessageLength];

public:
	FIoStatus(EIoErrorCode code, FErrorMessage text)
	{
		this->ErrorCode = code;
		wcscpy_s(this->ErrorMessage, sizeof(ErrorMessage), text);
	}

private:


	EIoErrorCode	ErrorCode = EIoErrorCode::Ok;
	FErrorMessage	ErrorMessage;

	friend class FIoStatusBuilder;
};

namespace addresses {
	inline void* curl_easy_setopt;
	inline void* curl_setopt;
	inline void* request_exit_with_status;
	inline void* unsafeenvironnement;
	inline void* get_engine_version;
	inline void* should_check_pak;
	inline void* validate_container_signature;
	inline void* game_engine;
}

//native calls
namespace native {
	inline CURLcode(*curl_easy_setopt)(void*, CURLoption, ...);
	inline CURLcode(__fastcall *curl_setopt)(void*, int, va_list);
	inline void(__fastcall *request_exit_with_status)(bool, uint8_t);
	inline void* (*get_engine_version)();
	inline __int64(__fastcall* unsafe_environnement)(__int64* a1, char a2, __int64 a3, char a4);
	inline __int64(*should_check_pak)(void);
	inline void*(__fastcall *fiostatus_tostring)(void*, void*);
	inline FIoStatus(__fastcall* ValidateContainerSignature)(__int64 a1, __int64 a2, __int64 a3, __int64 a4, __int64 a5, __int64 a6);
}

namespace configuration {
	inline bool disableSSL = false;
	inline bool detourURL = false;
	inline bool useProxy = false;
	inline bool bypass_pak_checks = false;
	inline std::string forwardProxy;
	inline std::string forwardHost;
	inline std::string forwardPort;
}

class console {
public:
	console() {
		AllocConsole();
		freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
	}

	~console() {
		FreeConsole();
	}
};

//functions definitions

void freeze_all_threads(void);
void resume_all_threads(void);
void find_patterns(void);
void assign_natives(void);
void place_hooks(void);
void check_version();
void log_pointer(const char* pointerName, void* address, bool showOffset = false); //may be useless tbh

bool get_config_path(std::filesystem::path& out);
void read_config(std::filesystem::path config_path);

//natives wrappers
float GetEngineVersion();
CURLcode curl_setopt(void* handle, CURLoption opt, ...);

//hooks
static CURLcode hk_curl_easy_setopt(void*, CURLoption, ...);
static void hk_request_exit_with_status(bool, uint8_t);
static __int64 hk_unsafe_environnement(__int64* a1, char a2, __int64 a3, char a4);

#endif