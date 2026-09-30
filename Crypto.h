#pragma once

#include <windows.h>
#include <vector>
#include <string>

// Утилиты
std::string toUtf8(const std::wstring& w);           // wstring → UTF-8
std::wstring toHex(const std::vector<BYTE>& data);   // Байты → hex-строка
std::vector<BYTE> fromHex(const std::wstring& hex);  // hex-строка → байты

// Криптография
std::vector<BYTE> generateSalt(); // 16 случайных байт
std::vector<BYTE> sha256(const std::wstring& password, const std::vector<BYTE>& salt); // 32 байта хеш

// Вывод ключа из пароля
bool deriveKey(const std::wstring& password,
               const std::vector<BYTE>& salt,
               std::vector<BYTE>& outKey,
               std::wstring& outError);

// Шифрование данных с помощью AES-256-GCM.
bool aesEncrypt(const std::vector<BYTE>& key,
                const std::vector<BYTE>& plaintext,
                std::vector<BYTE>& outIv,
                std::vector<BYTE>& outCipher,
                std::wstring& outError);

bool aesDecrypt(const std::vector<BYTE>& key,
                const std::vector<BYTE>& iv,
                const std::vector<BYTE>& cipher,
                std::vector<BYTE>& outPlaintext,
                std::wstring& outError);



// ================== Мои коменты ==================
// PBKDF2 = Password - Based Key Derivation Function 2.
// Перевод: «Функция вывода ключа из пароля, версия 2».