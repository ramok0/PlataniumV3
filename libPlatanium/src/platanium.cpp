#include <platanium.hpp>
#include <tlhelp32.h>

bool platanium::exchange_to(const platanium::AuthClient auth_client)
{
	using namespace platanium::authentification;

	//exchange code does not exists on other auth managers
	if (!account::get_current_account()->get_descriptor().is_epic) return false;

	if (account::get_current_account()->get_client_id() == auth_client.first) return true;

	//cast current account to EpicGamesAccount
	account::EpicGamesAccount* account = (account::EpicGamesAccount*)account::get_current_account().get();

	std::string exchange_code = account->get_exchange_code();

	if (exchange_code.size() == 0) return false;

	platanium::authentification::Credentials credentials;
	credentials.exchange_code = exchange_code;
	credentials.client_id = auth_client.first;
	credentials.type = EPIC_EXCHANGE_CODE;

	return platanium::login(credentials);
}


bool platanium::login(const platanium::authentification::Credentials credentials)
{
	std::shared_ptr<platanium::authentification::managers::BaseAuthManager> auth_manager = platanium::get_auth_manager(credentials.type);
	if (!auth_manager)
	{
		error::set_last_error(error::NO_EXISTS);
		spdlog::warn("failed to find auth manager");
		return false;
	}
	
	spdlog::debug("Using AuthManager : {}", auth_manager->get_name());

	std::shared_ptr<platanium::authentification::account::Account> account = auth_manager->login(credentials);
	if (!account)
	{
		error::set_last_error(error::INVALID_CREDENTIALS);
		return false;
	}

	platanium::authentification::account::set_current_account(account);

	return true;
}



bool platanium::initialize(void)
{
	static std::array<std::wstring, 6> kill_process = { L"EpicGamesLauncher.exe", L"FortniteLauncher.exe", L"FortniteClient-Win64-Shipping.exe",L"FortniteClient-Win64-Shipping_EAC.exe", L"FortniteClient-Win64-Shipping_BE.exe", L"FortniteClient-Win64-Shipping_EAC_EOS.exe" };

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(entry);

	if (Process32First(hSnapshot, &entry))
	{
		while (Process32Next(hSnapshot, &entry))
		{
			for (auto& process_name : kill_process)
			{
				if (std::wstring(entry.szExeFile).contains(process_name))
				{
					HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, false, entry.th32ProcessID);
					TerminateProcess(hProcess, 0);
					CloseHandle(hProcess);
				}
			}
		}
	}

	CloseHandle(hSnapshot);



	platanium::error::set_last_error(error::PLATANIUM_SUCCESS);
	spdlog::set_level(spdlog::level::trace);

	platanium::auth_managers.push_back(std::make_shared<platanium::authentification::managers::EpicAuthorizationCodeAuthManager>());
	platanium::auth_managers.push_back(std::make_shared<platanium::authentification::managers::EpicRefreshTokenAuthManager>());
	platanium::auth_managers.push_back(std::make_shared<platanium::authentification::managers::EpicExchangeCodeAuthManager>());
	platanium::auth_managers.push_back(std::make_shared<platanium::authentification::managers::EpicDeviceCodeAuthManager>());
	platanium::auth_managers.push_back(std::make_shared<platanium::authentification::managers::EpicDeviceAuthAuthManager>());

	platanium::auth_clients.push_back(platanium::epic::api::auth_clients::fortniteIOSGameClient);
	platanium::auth_clients.push_back(platanium::epic::api::auth_clients::launcherAppClient2);
	platanium::auth_clients.push_back(platanium::epic::api::auth_clients::fortniteNewSwitchGameClient);

	platanium::configuration = std::make_unique<Configuration>(std::filesystem::current_path() / "config.json");

	platanium::authentification::account::set_current_account(nullptr);

	spdlog::info("libPlatanium is initialized successfully");
	return true;
}

//i guess something here makes everything crash
bool platanium::destroy(void)
{
	platanium::auth_managers.clear();
	platanium::auth_clients.clear();
	if (platanium::configuration.get() != nullptr)
	{
		platanium::configuration = nullptr;
	}
	if (platanium::currentAccount.get() != nullptr)
	{
		platanium::currentAccount = nullptr;
	}

	spdlog::info("libPlatanium has been destroyed successfully");

	return true;
}
