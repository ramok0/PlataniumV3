#pragma once
#ifndef __cplusplus
#error This code should be compiled with a C++23 compiler
#endif

#include <filesystem>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <cpr/cpr.h>
#include <winternl.h>
#include <base64/base64.hpp>

#pragma comment(lib, "Version.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Crypt32.lib")

typedef NTSTATUS(WINAPI* PFN_NT_QUERY_SYSTEM_INFORMATION)(
	SYSTEM_INFORMATION_CLASS SystemInformationClass,
	PVOID                    SystemInformation,
	ULONG                    SystemInformationLength,
	PULONG                   ReturnLength
	);

typedef NTSTATUS(WINAPI* PFN_NT_SUSPEND_PROCESS)(
	HANDLE ProcessHandle
	);

typedef NTSTATUS(WINAPI* PFN_NT_RESUME_PROCESS)(
	HANDLE ProcessHandle
	);

namespace platanium {
	enum TOKEN_METHOD {
		EPIC_AUTHORIZATION_CODE,
		EPIC_REFRESH_TOKEN,
		EPIC_EXCHANGE_CODE,
		EPIC_DEVICE_CODE,
		METHOD_UNKNOWN
	};

	typedef std::pair<std::string, std::string> Header;
	typedef std::vector<Header> HeaderContainer;
	typedef std::pair<const char*, const char*> AuthClient;
	typedef std::pair<std::string, std::optional<std::string>> Argument;
	typedef std::vector<Argument> ArgumentContainer;

	namespace windows {
		NTSTATUS NtQueryStatusInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass,
			PVOID                    SystemInformation,
			ULONG                    SystemInformationLength,
			PULONG                   ReturnLength);

		NTSTATUS NtSuspendProcess(HANDLE ProcessHandle);
		NTSTATUS NtResumeProcess(HANDLE ProcessHandle);
	}

	namespace error {
		enum PLATANIUM_STATUS : uint8_t {
			PLATANIUM_SUCCESS,
			ALREADY_EXISTS,
			FAILED_TO_READ_FILE,
			FAILED_TO_FIND_FILE,
			FAILED_TO_WRITE_FILE,
			FAILED_TO_START_PROCESS,
			FAILED_TO_UNCIPHER_DATA,
			FAILED_TO_CIPHER_DATA,
			DOES_NOT_SUPPORT_SAVING,
			OUT_IS_NULLPTR,
			NO_EXISTS,
			INVALID_API_RESPONSE,
			INVALID_CREDENTIALS,
			MISSING_ENTRY,
			JSON_PARSING_ERROR
		};

		inline error::PLATANIUM_STATUS last_error;

		void set_last_error(error::PLATANIUM_STATUS error);
		error::PLATANIUM_STATUS get_last_error(void);
	}

	namespace json {
		bool json_exists(const nlohmann::json& doc, const std::string& key);

		template<typename T> bool extract_json(const nlohmann::json& doc, const std::string key, T* out);
	}

	namespace authentification {

		struct ExchangeCode {
			std::string exchange_code;
		};

		struct AuthorizationCode {
			std::string authorization_code;
		};

		struct RefreshToken {
			std::string refresh_token;
		};	
		
		struct DeviceCode {
			std::string device_code;
		};

		struct EpicGamesAccount {
			std::string client_id;
		};

		struct Credentials : public AuthorizationCode, public RefreshToken, public ExchangeCode, public DeviceCode, public EpicGamesAccount {
			platanium::TOKEN_METHOD type;

			nlohmann::json to_json(void);

			operator bool() {
				return !refresh_token.empty() || !authorization_code.empty() || !exchange_code.empty() || !client_id.empty() || !device_code.empty();
			}

			static bool from(const nlohmann::json& in, Credentials& out);

			bool cipher(std::string& out);
			static bool uncipher(const std::string in, Credentials& out);
		};

		namespace account {
			struct Details {
				std::string display_name;
				std::string access_token;
				std::string refresh_token;
				std::string account_id;

				bool is_valid() {
					return !display_name.empty();
				}

				operator bool() {
					return this->is_valid();
				}

				static bool from(const nlohmann::json& doc, Details& out);

				nlohmann::json to_json(void);
			};

			struct AccountDescriptor {
				Details details;
				std::string authManagerName;
				bool unique_authorization;
				std::string client_id;
				platanium::TOKEN_METHOD method;

				nlohmann::json to_json(void);

				operator bool() {
					return this->details;
				}
			};
			
			class Account {
			public:
				Account(const AccountDescriptor descriptor, Details details) {
					this->m_descriptor = descriptor;
					this->m_details = details;
				}

				const bool is_valid() {
					return this->m_descriptor.details;
				}

				virtual platanium::ArgumentContainer get_authentification_arguments() {
					return {};
				}

				virtual bool save();

				//array<header name, header value>
				virtual const platanium::HeaderContainer get_authentification_headers() {
					return {
						std::make_pair("Authorization", "Bearer " + this->m_details.access_token)
					};
				}

				virtual bool refresh() {
					spdlog::warn("You are calling BaseClass");
					return false;
				}

				const Details get_details() {
					return this->m_details;
				}	


				std::string get_client_id() {
					return this->m_descriptor.client_id;
				}
				
				const AccountDescriptor get_descriptor() {
					return this->m_descriptor;
				}

				virtual void set_details(const Details details)
				{
					this->m_details = details;
					this->m_descriptor.details = details;
				}

				virtual const Credentials get_refresh_token() {
					Credentials credentials;

					credentials.refresh_token = this->get_descriptor().details.refresh_token;
					credentials.client_id = this->get_client_id();
					
					return credentials;
				}

			protected:
				AccountDescriptor m_descriptor;
				Details m_details;
			};

			class EpicGamesAccount : public Account {
			public:
				EpicGamesAccount(AccountDescriptor descriptor) : Account(descriptor, descriptor.details) {
					
				}

				struct Caldera {
					std::string provider;
					std::string jwt;

					static const Caldera from(const nlohmann::json& doc);

					operator bool() {
						return !jwt.empty();
					}
				};

				const Caldera request_anti_cheat_provider(void);
				const std::string get_exchange_code(void);


				const Credentials get_refresh_token() override;

				platanium::ArgumentContainer get_authentification_arguments() override;

				bool refresh() override;
			};

			bool is_account_valid(void);
			std::shared_ptr<Account> get_current_account(void);
			void set_current_account(std::shared_ptr<Account> account);
		}

		namespace managers {
			class BaseAuthManager {
			public:
				BaseAuthManager(const std::string& name)
					: m_name(name)
				{
					this->m_type = platanium::METHOD_UNKNOWN;
					spdlog::info("Created AuthManager {} successfully", this->get_name());
				}

				virtual bool is_official() {
					return this->m_name.contains("Epic");
				}

				virtual std::shared_ptr<platanium::authentification::account::Account> login(const Credentials& creds) {
					throw std::runtime_error("login not implemented for this auth manager");
				}

				const std::string get_name() {
					return this->m_name;
				}	

				virtual const platanium::TOKEN_METHOD get_method() {
					return this->m_type;
				}
			
			private:
				std::string m_name;
			protected:
				platanium::TOKEN_METHOD m_type;
				bool bSupportsMultipleConnections = false;
			};

			class EpicAuthManager : public BaseAuthManager {
			public:
				EpicAuthManager(const std::string& name) : BaseAuthManager(name) { };

				std::shared_ptr<platanium::authentification::account::Account> epic_login(std::string token, std::string client_id);
			};

			class EpicAuthorizationCodeAuthManager : public EpicAuthManager {
			public:
				EpicAuthorizationCodeAuthManager() : EpicAuthManager("EpicAuthorizationCodeAuthManager") {
					this->m_type = platanium::EPIC_AUTHORIZATION_CODE;
				};

				std::shared_ptr<platanium::authentification::account::Account> login(const Credentials& creds) override;
			};

			class EpicRefreshTokenAuthManager : public EpicAuthManager {
			public:
				EpicRefreshTokenAuthManager() : EpicAuthManager("EpicRefreshTokenAuthManager") {
					this->m_type = platanium::EPIC_REFRESH_TOKEN;
					this->bSupportsMultipleConnections = true;
				};

				std::shared_ptr<platanium::authentification::account::Account> login(const Credentials& creds) override;
			};		
			
			class EpicExchangeCodeAuthManager : public EpicAuthManager {
			public:
				EpicExchangeCodeAuthManager() : EpicAuthManager("EpicExchangeCodeAuthManager") {
					this->m_type = platanium::EPIC_EXCHANGE_CODE;
				};

				std::shared_ptr<platanium::authentification::account::Account> login(const Credentials& creds) override;
			};

			class EpicDeviceCodeAuthManager : public EpicAuthManager {
			public:
				EpicDeviceCodeAuthManager() : EpicAuthManager("EpicExchangeCodeAuthManager") {
					this->m_type = platanium::EPIC_DEVICE_CODE;
				};

				std::shared_ptr<platanium::authentification::account::Account> login(const Credentials& creds) override;
			};
		}


	}

	class Configuration {
	public:
		struct Data {
			bool bUseProxy;
			bool bRedirectTraffic;
			std::string proxy;
			std::string url;
			uint16_t port;
			std::string fortnite_path;
		};

		Configuration(std::filesystem::path configuration_path) {
			this->m_config_path = configuration_path;
			bool load_success = this->exists() ? this->load() : this->load_default();

			if (!load_success) {
				throw std::runtime_error("failed to load configuration");
			}

			spdlog::info("Configuration is loaded successfully");
		}

		~Configuration() {
			if (this->data != nullptr) {
				delete this->data;
			}

			spdlog::info("Configuration has been destroyed successfully");
		}

		Data* get() {
			return this->data;
		}

		const std::filesystem::path get_path()
		{
			return this->m_config_path;
		}

		bool write();

		operator bool() {
			return this->data != nullptr;
		}

	private:
		Data get_default();

		bool load();

		bool load_default();

		bool exists();

		bool read(std::string& out);

		bool parse(const std::string& str);

		bool extract_fields(const nlohmann::json& doc);

		Data* data;
		std::filesystem::path m_config_path;
	};

	namespace epic {
		namespace api {
			namespace endpoints {
				constexpr const char* TOKEN = "https://account-public-service-prod.ol.epicgames.com/account/api/oauth/token";
				constexpr const char* EXCHANGE = "https://account-public-service-prod.ol.epicgames.com/account/api/oauth/exchange";
				constexpr const char* RACP = "https://caldera-service-prod.ecosec.on.epicgames.com/caldera/api/v1/launcher/racp";
				constexpr const char* CREATE_DEVICE_CODE = "https://account-public-service-prod.ol.epicgames.com/account/api/oauth/deviceAuthorization";
			}

			namespace auth_clients {
				constexpr AuthClient fortniteIOSGameClient = std::make_pair("3446cd72694c4a4485d81b77adbb2141", "9209d4a5e25a457fb9b07489d313b41a");
				constexpr AuthClient launcherAppClient2 = std::make_pair("34a02cf8f4414e29b15921876da36f9a", "daafbccc737745039dffe53d94fc76cf");
				constexpr AuthClient fortniteNewSwitchGameClient = std::make_pair("98f7e42c2e3a4f86a74eb43fbb41ed39", "0a2449a2-001a-451e-afec-3e812901c4d7");

				inline std::string build(AuthClient code)
				{
					return base64_encode(std::string(code.first) + ":" + std::string(code.second));
				}
			}

			enum METHOD {
				GET,
				POST,
				PATCH
			};

			namespace account {
				namespace authentfication {
					struct DeviceCode {
						std::string user_code;
						std::string device_code;
						std::string client_id;
					};

					struct ClientCredentials {
						std::string user_code;
						std::string device_code;
						std::string verification_uri_complete;
						std::chrono::system_clock::time_point expires_at;

						inline bool is_expired() const {
							return std::chrono::system_clock::now() > expires_at;
						}
					};

					static std::map<TOKEN_METHOD, std::pair<std::string, std::string>> token_to_body = {
						{EPIC_AUTHORIZATION_CODE, std::make_pair("authorization_code", "code")},
						{EPIC_REFRESH_TOKEN, std::make_pair("refresh_token", "refresh_token")},
						{EPIC_EXCHANGE_CODE, std::make_pair("exchange_code", "exchange_code")},
						{EPIC_DEVICE_CODE, std::make_pair("device_code", "device_code")},
					};

					std::optional<platanium::epic::api::account::authentfication::DeviceCode> get_device_code(void);

					//returns a token for client_credentials
					const std::optional<std::string> client_credentials(const AuthClient client);

					//creates a device code for authentification
					const std::optional<ClientCredentials> create_device_code(void);

					platanium::authentification::account::AccountDescriptor token(const std::string token, const AuthClient client, TOKEN_METHOD token_type);
					const std::string exchange(const std::string& access_token);
				}
			}

			namespace caldera {
				const platanium::authentification::account::EpicGamesAccount::Caldera racp(const std::string& account_id, const std::string& exchange_code, const std::string epic_app);
			}

			cpr::Response request(const std::string url, HeaderContainer& headers, METHOD method, const std::string body);
		}

		class LauncherInstalled {
		public:
			struct LauncherInstalledEntry {
				std::string InstallLocation;
				std::string NamespaceId;
				std::string ItemId;
				std::string ArtifactId;
				std::string AppVersion;
				std::string AppName;

				static LauncherInstalledEntry from(const nlohmann::json& doc);
			};

			LauncherInstalled() {
				if (!this->parse())
				{
					spdlog::warn("Platanium wont be able to find Fortnite installation automaticly.");
				}
			}

			bool find_by_app_name(const std::string& app_name, LauncherInstalledEntry& out);

			static bool find_fortnite_path(std::filesystem::path& out);
		private:
			bool exists(void);

			bool read(std::string& out);

			bool parse(void);

			bool find(LauncherInstalledEntry& out, std::function<bool(const LauncherInstalledEntry&)> prediction);

			std::filesystem::path m_path = std::filesystem::path("C:\\ProgramData\\Epic\\UnrealEngineLauncher\\LauncherInstalled.dat");
			std::vector<LauncherInstalledEntry> m_data;
		};

		class FortniteHandle {
		public:
			FortniteHandle(STARTUPINFOA startup_info, PROCESS_INFORMATION process_info)
			{
				this->m_process_info = process_info;
				this->m_startup_info = startup_info;

				CloseHandle(this->get_process_info()->hThread);
			}

			~FortniteHandle() {
				if (this->is_alive())
				{
					spdlog::info("Closing Fortnite because FortniteHandle destructor has been called");
					this->kill();
				}

				if (this->get_process_info()->hProcess != INVALID_HANDLE_VALUE) {
					CloseHandle(this->get_process_info()->hProcess);
				}
			}

			inline void kill() {
				TerminateProcess(this->get_process_info()->hProcess, 0);
			}

			bool is_alive(void);
			bool is_suspended(void);

			bool suspend(void);
			bool resume(void);

			bool inject(std::filesystem::path dll_path);

			STARTUPINFOA* get_startup_info() {
				return &this->m_startup_info;
			}

			PROCESS_INFORMATION* get_process_info() {
				return &this->m_process_info;
			}



		private:
			STARTUPINFOA m_startup_info;
			PROCESS_INFORMATION m_process_info;
		};

		class Fortnite {
		public:
			Fortnite(std::filesystem::path fortnite_path) {
				this->m_fortnite_path = fortnite_path;
			}

			static std::string get_start_arguments(platanium::ArgumentContainer override_arguments);

			static std::filesystem::path find_path(void);

			inline std::filesystem::path get_binary_path(void)
			{
				return this->m_fortnite_path / "FortniteGame" / "Binaries" / "Win64" / "FortniteClient-Win64-Shipping.exe";
			}

			bool exists(void);

			float get_engine_version(void);

			FortniteHandle* start(const std::string& arguments);
		private:
			std::filesystem::path m_fortnite_path;
		};
	}

	std::shared_ptr<authentification::managers::BaseAuthManager> get_auth_manager(const std::string authManagerName);
	std::shared_ptr<authentification::managers::BaseAuthManager> get_auth_manager(platanium::TOKEN_METHOD authManagerType);

	bool login(const platanium::authentification::Credentials credentials);

	//you should call this function before using platanium
	extern bool initialize(void);
	//you should call this function when you dont need platanium anymore
	extern bool destroy(void);

	inline std::vector<std::shared_ptr<authentification::managers::BaseAuthManager>> auth_managers;
	inline std::vector<std::pair<const char*, const char*>> auth_clients;
	inline std::shared_ptr<platanium::authentification::account::Account> currentAccount;
	inline std::vector<std::pair<std::string, platanium::authentification::Credentials>> accounts;
	inline std::unique_ptr<Configuration> configuration;
}

template<typename T>
bool platanium::json::extract_json(const nlohmann::json& doc, const std::string key, T* out)
{
	if (!out) {
		spdlog::error("out is nullptr, failed to extract json");
		set_last_error(error::OUT_IS_NULLPTR);
		return false;
	}

	if (!json::json_exists(doc, key)) {
		spdlog::error("failed to extract key {}", key);
		set_last_error(error::NO_EXISTS);
		return false;
	}

	*out = doc[key].get<T>();

	return true;
}