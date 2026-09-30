#include "Globals.h"

// ================== Константы ==================
const wchar_t* USERS_FILE = L"users.dat";
const wchar_t* LOGIN_CLASS_NAME = L"LoginClass";
const wchar_t* CABINET_CLASS_NAME = L"CabinetClass";
const wchar_t* NOTES_FOLDER = L"notes";

// ================== Глобальные переменные ==================
HWND hLoginField = NULL;
HWND hPasswordField = NULL;
HWND hStatusLabel = NULL;
HWND hNotesField = NULL;
HWND hCabinetTitle = NULL;

HWND g_hLoginWindow = NULL;
HWND g_hCabinetWindow = NULL;

std::wstring g_currentUser;
std::wstring g_currentPassword;