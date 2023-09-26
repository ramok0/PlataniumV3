//#include "../include/internationalization.h"
//#include <Windows.h>
//
//constexpr const char* REGISTERY_KEY = "SOFTWARE\\PLATANIUMV3";
//constexpr const char* REGISTERY_VALUE_NAME = "Language";
//
//void Internationalization::retrive_language()
//{
//    HKEY hKey;
//
//    DWORD dwValue;
//    DWORD dwType;
//    DWORD dwSize = sizeof(DWORD);
//
//    LONG openResult = RegCreateKeyExA(HKEY_CURRENT_USER, REGISTERY_KEY, 0, NULL, 0, KEY_READ | KEY_WRITE, NULL, &hKey, NULL);
//    if (openResult != ERROR_SUCCESS) return;
//
//    LONG queryResult = RegQueryValueExA(hKey, REGISTERY_VALUE_NAME, NULL, &dwType, (LPBYTE)&dwValue, &dwSize);
//    if (queryResult != ERROR_SUCCESS || dwType != REG_DWORD) {
//        RegCloseKey(hKey);
//        return;
//    }
//
//    LANGUAGES language = (LANGUAGES)dwValue;
//
//    this->language = language;
//
//    RegCloseKey(hKey);
//}
//
//void Internationalization::write_language()
//{
//    HKEY hKey;
//
//    LONG openResult = RegCreateKeyExA(HKEY_CURRENT_USER, REGISTERY_KEY, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
//    if (openResult != ERROR_SUCCESS) {
//        return;
//    }
//    
//    RegSetValueExA(hKey, REGISTERY_VALUE_NAME, 0, REG_DWORD, (const BYTE*)&this->language, sizeof(DWORD));
//    RegCloseKey(hKey);
//}
