#include <platanium.hpp>

std::string platanium::epic::Fortnite::get_start_arguments(platanium::ArgumentContainer override_arguments)
{
	static std::array<std::pair<std::string, std::optional<std::string>>, 13> default_arguments {
		std::make_pair(xorstr_("EpicPortal"), std::nullopt),
		std::make_pair(xorstr_("AUTH_LOGIN"), xorstr_("unused")),
		std::make_pair(xorstr_("AUTH_PASSWORD"), std::nullopt),
		std::make_pair(xorstr_("AUTH_TYPE"), xorstr_("exchangecode")),
		std::make_pair(xorstr_("epicapp"), xorstr_("Fortnite")),
		std::make_pair(xorstr_("epicenv"), xorstr_("Prod")),
		std::make_pair(xorstr_("epicusername"), std::nullopt),
		std::make_pair(xorstr_("epicuserid"), std::nullopt),
		std::make_pair(xorstr_("fromfl"), std::nullopt),
		std::make_pair(xorstr_("caldera"), std::nullopt),
		std::make_pair(xorstr_("epicsandboxid"), xorstr_("fn")),
		std::make_pair(xorstr_("plataniumconfigpath"), std::nullopt),
		std::make_pair(xorstr_("obfuscationid"), "Epy0GaQU3WUOcTmtT0--0P2_vOixxg"),
	};

	platanium::ArgumentContainer arguments;
	std::copy(default_arguments.begin(), default_arguments.end(), std::back_inserter(arguments));

	for (auto& override_arg : override_arguments) {
		auto it = std::find_if(arguments.begin(), arguments.end(),
			[&override_arg](const platanium::Argument& arg) {
				return arg.first == override_arg.first;
			});

		if (it != arguments.end()) {
			it->second = override_arg.second;
		}
		else {
			arguments.push_back(override_arg);
		}
	}

	std::ostringstream out;

	for (const auto& argument : arguments) {
		out << xorstr_("-") << argument.first;
		if (argument.second.has_value()) {
			out << xorstr_("=") << argument.second.value();
		}
		out << " ";
	}

	std::string outStr = out.str();

	spdlog::info("Arguments for fortnite : {}", outStr);

	return outStr;
}

std::filesystem::path platanium::epic::Fortnite::find_path(void)
{
	std::filesystem::path out;
	if (!platanium::epic::LauncherInstalled::find_fortnite_path(out)) return out;
	
	return out;
}

bool platanium::epic::Fortnite::exists(void)
{
	return std::filesystem::exists(this->m_fortnite_path / xorstr_("Engine")) && std::filesystem::exists(this->m_fortnite_path / xorstr_("FortniteGame")) && std::filesystem::exists(this->get_binary_path());
}

float platanium::epic::Fortnite::get_engine_version(void)
{
	if (!this->exists()) return 0.f;
	DWORD dwHandle = 0;

	DWORD dwSize = GetFileVersionInfoSizeA(this->m_fortnite_path.string().c_str(), &dwHandle);
	if (dwSize == NULL)
	{
		spdlog::error(xorstr_("Failed to get Engine Version"));
		return 0.f;
	}

	std::vector<BYTE> buffer(dwSize);
	if (!GetFileVersionInfoA(this->m_fortnite_path.string().c_str(), NULL, dwSize, buffer.data()))
	{
		spdlog::error(xorstr_("Failed to get Engine Version"));
		return 0.f;
	}

	VS_FIXEDFILEINFO* pFileInfo;
	UINT uLen;
	if (!VerQueryValueA(buffer.data(), "\\", (LPVOID*)&pFileInfo, &uLen))
	{
		spdlog::error(xorstr_("Failed to get Engine Version"));
		return 0.f;
	}

	spdlog::trace("Fortnite file version: {}.{}.{}.{}", HIWORD(pFileInfo->dwFileVersionMS), LOWORD(pFileInfo->dwFileVersionMS), HIWORD(pFileInfo->dwFileVersionLS), LOWORD(pFileInfo->dwFileVersionLS));

	DWORD firstPart = HIWORD(pFileInfo->dwFileVersionMS);
	DWORD secondPart = LOWORD(pFileInfo->dwFileVersionMS);

	std::string strResult = std::format("{}.{}", firstPart, secondPart);

	return std::stof(strResult);
}

platanium::epic::FortniteHandle* platanium::epic::Fortnite::start(const std::string& arguments)
{
	if (!this->exists())
	{
		spdlog::error(xorstr_("Failed to find game binary"));
		error::set_last_error(error::FAILED_TO_FIND_FILE);
		return nullptr;
	}

	auto generateArgs = [](const std::string& binary_name, const std::string& args) {
		return "\"" + binary_name + "\" " + args;
	};

	platanium::epic::utils::kill_anticheats();

	std::filesystem::path binary_path = this->get_binary_path();

	STARTUPINFOA startup_info;
	PROCESS_INFORMATION process_info;

	ZeroMemory(&startup_info, sizeof(STARTUPINFOA));
	ZeroMemory(&process_info, sizeof(PROCESS_INFORMATION));

	STARTUPINFOA antiCheatStartupInfo;
	PROCESS_INFORMATION antiCheatProcessInfo;

	ZeroMemory(&antiCheatStartupInfo, sizeof(STARTUPINFOA));
	ZeroMemory(&antiCheatProcessInfo, sizeof(PROCESS_INFORMATION));

	std::filesystem::path parent_path = binary_path.parent_path();

	if (!CreateProcessA(NULL, (char*)generateArgs(binary_path.string(), arguments).c_str(), nullptr, nullptr, false, CREATE_SUSPENDED | DETACHED_PROCESS, NULL, parent_path.string().c_str(), &startup_info, &process_info))
	{
		error::set_last_error(error::FAILED_TO_START_PROCESS);
		spdlog::error("Failed to start process, OS error code : {}", GetLastError());
		return nullptr;
	}

	const std::array<std::string, 4> anti_cheats_paths = { xorstr_("FortniteClient-Win64-Shipping_BE.exe"),xorstr_("FortniteClient-Win64-Shipping_EAC_EOS.exe"),xorstr_("FortniteClient-Win64-Shipping_EAC.exe"),xorstr_("FortniteLauncher.exe")};

	for (auto& path : anti_cheats_paths)
	{
		if (!CreateProcessA(NULL, (char*)generateArgs((parent_path / path).string(), arguments).c_str(), nullptr, nullptr, false, CREATE_SUSPENDED | DETACHED_PROCESS, NULL, parent_path.string().c_str(), &antiCheatStartupInfo, &antiCheatProcessInfo))
		{
			continue;
		}

		CloseHandle(antiCheatProcessInfo.hProcess);
		CloseHandle(antiCheatProcessInfo.hThread);
	}

	platanium::epic::FortniteHandle* handle = new platanium::epic::FortniteHandle(startup_info, process_info);
	std::this_thread::sleep_for(std::chrono::milliseconds(700));
	return handle;
}