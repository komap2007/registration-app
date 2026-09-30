#include "Crypto.h"
#include <bcrypt.h>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "bcrypt.lib") // подключить bcrypt.lib

// ================== Утилиты ==================

std::string toUtf8(const std::wstring& w) {
    if (w.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), NULL, 0, NULL, NULL);
    std::string out(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &out[0], len, NULL, NULL);
    return out;
}

std::wstring toHex(const std::vector<BYTE>& data) {
    std::wstringstream ss;
    for (BYTE b : data)
        ss << std::hex << std::setw(2) << std::setfill(L'0') << (int)b;
    return ss.str();
}

std::vector<BYTE> fromHex(const std::wstring& hex) {
    std::vector<BYTE> out;
    for (size_t i = 0; i + 1 < hex.size(); i += 2) {
        BYTE b = (BYTE)wcstol(hex.substr(i, 2).c_str(), nullptr, 16);
        out.push_back(b);
    }
    return out;
}

// ================== Хеширование ==================

std::vector<BYTE> generateSalt() {
    std::vector<BYTE> salt(16);
    NTSTATUS status = BCryptGenRandom(NULL, salt.data(), (ULONG)salt.size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (status != 0)
        salt.clear();
    return salt;
}

std::vector<BYTE> sha256(const std::wstring& password, const std::vector<BYTE>& salt) {
    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_HASH_HANDLE hHash = NULL;
    std::vector<BYTE> hash(32);
    DWORD hashLen = 0, cbData = 0;
    NTSTATUS status;

    // Открыть провайдер SHA-256
    status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, NULL, 0);
    if (status != 0) return hash;
    
    // Узнать размер служебного буфера
    status = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PUCHAR)&cbData, sizeof(DWORD), &hashLen, 0);
    if (status != 0) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return hash;
    }

    // Создать сулжебный буфер нужного размера
    std::vector<BYTE> hashObj(cbData);
    status = BCryptCreateHash(hAlg, &hHash, hashObj.data(), cbData, NULL, 0, 0);
    if (status != 0) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return hash;
    }

    std::string password_utf8 = toUtf8(password);

    // Скормить пароль
    status = BCryptHashData(hHash, (PUCHAR)password_utf8.data(), (ULONG)password_utf8.size(), 0);
    if (status != 0) {
        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return hash;
    }

    // Скормить соль
    status = BCryptHashData(hHash, (PUCHAR)salt.data(), (ULONG)salt.size(), 0);
    if (status != 0) {
        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return hash;
    }

    // Финал создания хэша
    status = BCryptFinishHash(hHash, hash.data(), (ULONG)hash.size(), 0);
    if (status != 0) {
        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return hash;
    }

    BCryptDestroyHash(hHash);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    return hash;
}

bool deriveKey(const std::wstring& password, const std::vector<BYTE>& salt, std::vector<BYTE>& outKey, std::wstring& outError) {
    BCRYPT_ALG_HANDLE hAlg = NULL;

    NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, NULL, BCRYPT_ALG_HANDLE_HMAC_FLAG);
    if (status != 0) {
        outError = L"Не удалось открыть провайдер SHA-256 для PBKDF2.";
        return false;
    }

    std::string passwordUtf8 = toUtf8(password);
    outKey.resize(32);

    status = BCryptDeriveKeyPBKDF2(hAlg, (PUCHAR)passwordUtf8.data(), (ULONG)passwordUtf8.size(), (PUCHAR)salt.data(), (ULONG)salt.size(), 100000, outKey.data(), (ULONG)outKey.size(), 0);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    if (status != 0) {
        outError = L"Не удалось вывести ключ из пароля.";
        return false;
    }

    return true;
}

// plaintext — открытый текст (байты).
// outIv — выход : вектор инициализации(12 байт).
// I = Initialization (инициализация).
// V = Vector(вектор).
// outCipher — выход : шифротекст.
// outError — выход : сообщение об ошибке.
bool aesEncrypt(const std::vector<BYTE>& key, const std::vector<BYTE>& plaintext, std::vector<BYTE>& outIv, std::vector<BYTE>& outCipher, std::wstring& outError) {
    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_KEY_HANDLE hKey = NULL;

    NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0);
    if (status != 0) { 
        outError = L"Не удалось открыть AES."; 
        return false; }

    // Устанавливка режима GCM
    status = BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_GCM, sizeof(BCRYPT_CHAIN_MODE_GCM), 0);
    if (status != 0) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        outError = L"Не удалось установить режим GCM.";
        return false;
    }

    // Создать ключ из 32 байт
    status = BCryptGenerateSymmetricKey(hAlg, &hKey, NULL, 0, (PUCHAR)key.data(), (ULONG)key.size(), 0);
    if (status != 0) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        outError = L"Не удалось создать AES-ключ.";
        return false;
    }

    // Генерирация IV
    outIv.resize(12);
    status = BCryptGenRandom(NULL, outIv.data(), (ULONG)outIv.size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (status != 0) {
        BCryptDestroyKey(hKey);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        outError = L"Не удалось сгенерировать IV.";
        return false;
    }

    // Буфер для тега аутентификации
    std::vector<BYTE> tag(16);

    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO authInfo;
    BCRYPT_INIT_AUTH_MODE_INFO(authInfo);
    authInfo.pbNonce = outIv.data();
    authInfo.cbNonce = (ULONG)outIv.size();
    authInfo.pbTag = tag.data();
    authInfo.cbTag = 16;

    ULONG cipherSize = 0;
    status = BCryptEncrypt(hKey, (PUCHAR)plaintext.data(), (ULONG)plaintext.size(), &authInfo, NULL, 0, NULL, 0, &cipherSize, 0);
    if (status != 0) {
        BCryptDestroyKey(hKey);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        outError = L"Не удалось вычислить размер шифра.";
        return false;
    }

    outCipher.resize(cipherSize);
    status = BCryptEncrypt(hKey, (PUCHAR)plaintext.data(), (ULONG)plaintext.size(), &authInfo, NULL, 0, outCipher.data(), (ULONG)outCipher.size(), &cipherSize, 0);

    BCryptDestroyKey(hKey);
    BCryptCloseAlgorithmProvider(hAlg, 0);

    if (status != 0) {
        outError = L"Не удалось зашифровать данные.";
        return false;
    }

    outCipher.insert(outCipher.end(), tag.begin(), tag.end());
    return true;
}

bool aesDecrypt(const std::vector<BYTE>& key, const std::vector<BYTE>& iv, const std::vector<BYTE>& cipher, std::vector<BYTE>& outPlaintext, std::wstring& outError) {
    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_KEY_HANDLE hKey = NULL;

    NTSTATUS status = BCryptOpenAlgorithmProvider( &hAlg, BCRYPT_AES_ALGORITHM, NULL, 0);
    if (status != 0) { outError = L"Не удалось открыть AES."; return false; }

    status = BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_GCM, sizeof(BCRYPT_CHAIN_MODE_GCM), 0);
    if (status != 0) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        outError = L"Не удалось установить режим GCM.";
        return false;
    }

    status = BCryptGenerateSymmetricKey(hAlg, &hKey, NULL, 0, (PUCHAR)key.data(), (ULONG)key.size(), 0);
    if (status != 0) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        outError = L"Не удалось создать AES-ключ.";
        return false;
    }

    if (cipher.size() < 16) {
        BCryptDestroyKey(hKey);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        outError = L"Повреждённые данные.";
        return false;
    }

    std::vector<BYTE> tag(cipher.begin() + cipher.size() - 16, cipher.end());

    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO authInfo;
    BCRYPT_INIT_AUTH_MODE_INFO(authInfo);
    authInfo.pbNonce = (PUCHAR)iv.data();
    authInfo.cbNonce = (ULONG)iv.size();
    authInfo.pbTag = tag.data();
    authInfo.cbTag = 16;

    ULONG plainSize = 0;
    status = BCryptDecrypt(hKey, (PUCHAR)cipher.data(), (ULONG)(cipher.size() - 16), &authInfo, NULL, 0, NULL, 0, &plainSize, 0);
    if (status != 0) {
        BCryptDestroyKey(hKey);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        outError = L"Не удалось вычислить размер.";
        return false;
    }

    outPlaintext.resize(plainSize);
    status = BCryptDecrypt(hKey, (PUCHAR)cipher.data(), (ULONG)(cipher.size() - 16), &authInfo, NULL, 0, outPlaintext.data(), (ULONG)outPlaintext.size(), &plainSize, 0);

    BCryptDestroyKey(hKey);
    BCryptCloseAlgorithmProvider(hAlg, 0);

    if (status != 0) {
        outError = L"Не удалось расшифровать (неверный пароль или повреждённый файл).";
        return false;
    }
    outPlaintext.resize(plainSize);
    return true;
}