#include "Storage.h"
#include "Globals.h"
#include "Crypto.h"
#include <fstream>
#include <iterator>

// ================== users.dat ==================

bool userExists(const std::wstring& login) {
    std::ifstream f(USERS_FILE);
    if (!f.is_open()) return false;
    std::string line;
    std::string login_utf8 = toUtf8(login);
    while (std::getline(f, line)) {
        size_t tab = line.find('\t');
        if (tab != std::string::npos && line.substr(0, tab) == login_utf8)
            return true;
    }
    return false;
}

void saveUser(const std::wstring& login, const std::vector<BYTE>& salt, const std::vector<BYTE>& hash) {
    std::ofstream f(USERS_FILE , std::ios::app);
    if (!f.is_open()) return;
    f << toUtf8(login) << '\t' << toUtf8(toHex(salt)) << '\t' << toUtf8(toHex(hash)) << '\n';
    f.flush();
    f.close();
}

bool verifyUser(const std::wstring& login, const std::wstring& password) {
    std::ifstream f(USERS_FILE);
    if (!f.is_open()) return false;
    std::string line;
    std::string loginUtf8 = toUtf8(login);
    while (std::getline(f, line)) {
        size_t p1 = line.find('\t');
        size_t p2 = line.find('\t', p1 + 1);
        if (p1 == std::string::npos || p2 == std::string::npos) continue;

        std::string l = line.substr(0, p1);
        std::string saltHex = line.substr(p1 + 1, p2 - p1 - 1);
        std::string hashHex = line.substr(p2 + 1);

        if (l == loginUtf8) {
            std::wstring wsalt(saltHex.begin(), saltHex.end());
            std::wstring whash(hashHex.begin(), hashHex.end());

            std::vector<BYTE> salt = fromHex(wsalt);
            std::vector<BYTE> expected = fromHex(whash);
            std::vector<BYTE> actual = sha256(password, salt);
            return actual == expected;
        }
    }
    return false;
}

bool deleteUser(const std::wstring& login) {
    // 1. Читаем все строки в память
    std::ifstream fin(USERS_FILE);
    if (!fin.is_open()) return false;

    std::vector<std::string> lines;
    std::string line;
    std::string login_utf8 = toUtf8(login);
    bool found = false;

    while (std::getline(fin, line)) {
        // найти первый таб, до него стоит логин
        size_t tab = line.find('\t');
        if (tab != std::string::npos && line.substr(0, tab) == login_utf8) {
            found = true; // нашли - не сохраняем эту строчку
            continue;
        }
        lines.push_back(line); // остальные сохраняем 
    }
    fin.close();

    if (!found) return false;

    // 2. Перезаписываем файл (стираем всё)
    std::ofstream fout(USERS_FILE, std::ios::trunc); // std::ios::trunc — режим «обрезать» (стереть всё содержимое)
    if (!fout.is_open()) return false;

    // 3. Записываем оставшиеся строки
    for (const std::string& l : lines)
        fout << l << '\n';

    fout.flush();
    fout.close();
    return true;
}

// ================== notes/<login>.dat ==================

std::wstring getNotesPath(const std::wstring& login) {
    return std::wstring(NOTES_FOLDER ) + L"/" + login + L".dat";
}

std::wstring loadNotes(const std::wstring& login) {
    if (g_currentPassword.empty()) return L"";

    std::wstring path = getNotesPath(login);
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) return L"";

    std::vector<BYTE> fileData((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();

    if (fileData.size() < 16 + 12) return L"";

    std::vector<BYTE> salt(fileData.begin(), fileData.begin() + 16);
    std::vector<BYTE> iv(fileData.begin() + 16, fileData.begin() + 28);
    std::vector<BYTE> cipher(fileData.begin() + 28, fileData.end());

    std::vector<BYTE> key;
    std::wstring error;
    if (!deriveKey(g_currentPassword, salt, key, error)) return L"";

    std::vector<BYTE> plaintext;
    if (!aesDecrypt(key, iv, cipher, plaintext, error)) {
        return L"<Не удалось расшифровать. Неверный пароль или файл повреждён.>";
    }

    int wideLen = MultiByteToWideChar(CP_UTF8, 0, (char*)plaintext.data(), (int)plaintext.size(), NULL, 0);
    if (wideLen <= 0) return L"";
    std::wstring result(wideLen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, (char*)plaintext.data(), (int)plaintext.size(), &result[0], wideLen);
    return result;
}

bool saveNotes(const std::wstring& login, const std::wstring& text) {
    if (g_currentPassword.empty()) return false;

    CreateDirectoryW(NOTES_FOLDER , NULL);

    std::vector<BYTE> salt = generateSalt();

    std::vector<BYTE> key;
    std::wstring error;
    if (!deriveKey(g_currentPassword, salt, key, error)) return false;

    std::string utf8 = toUtf8(text);
    std::vector<BYTE> plaintext(utf8.begin(), utf8.end());

    std::vector<BYTE> iv, cipher;
    if (!aesEncrypt(key, plaintext, iv, cipher, error)) return false;

    std::wstring path = getNotesPath(login);
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open()) return false;

    f.write((char*)salt.data(), salt.size());
    f.write((char*)iv.data(), iv.size());
    f.write((char*)cipher.data(), cipher.size());

    f.flush();
    f.close();
    return true;
}

bool deleteNotes(const std::wstring& login) {
    std::wstring path = getNotesPath(login);
    if (DeleteFileW(path.c_str()))
        return true;

    DWORD err = GetLastError();
    if (err == ERROR_FILE_NOT_FOUND)
        return true;

    return false;
}