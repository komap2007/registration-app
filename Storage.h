#pragma once

#include <windows.h>
#include <string>
#include <vector>

// ================== Работа с users.dat ==================
bool userExists(const std::wstring& login);

void saveUser(const std::wstring& login,
              const std::vector<BYTE>& salt,
              const std::vector<BYTE>& hash);

bool verifyUser(const std::wstring& login,
                const std::wstring& password);

bool deleteUser(const std::wstring& login);

// ================== Работа с notes/<login>.dat ==================
std::wstring getNotesPath(const std::wstring& login);

std::wstring loadNotes(const std::wstring& login);

bool saveNotes(const std::wstring& login, const std::wstring& text);

bool deleteNotes(const std::wstring& login);