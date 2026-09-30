#pragma once

#include <windows.h>
#include <string>

// ================== ID элементов управления ==================

// --- Окно входа ---
#define ID_LOGIN_FIELD      1001   // поле ввода логина
#define ID_PASSWORD_FIELD   1002   // поле ввода пароля
#define ID_REGISTER_BUTTON  1003   // кнопка "Зарегистрироваться"
#define ID_LOGIN_BUTTON     1004   // кнопка "Войти"
#define ID_STATUS_LABEL     1005   // надпись статуса

// --- Окно кабинета ---
#define ID_NOTES_FIELD      2001   // поле заметок
#define ID_SAVE_BUTTON      2002   // кнопка "Сохранить"
#define ID_LOGOUT_BUTTON    2003   // кнопка "Выйти"
#define ID_CABINET_LABEL    2004   // надпись "Личный кабинет"
#define ID_DELETE_BUTTON	2005   // кнопка "Удалить аккаунт"

// ================== Константы ==================
extern const wchar_t* USERS_FILE ;          // файл с пользователями
extern const wchar_t* LOGIN_CLASS_NAME;     // класс окна входа
extern const wchar_t* CABINET_CLASS_NAME;   // класс окна кабинета
extern const wchar_t* NOTES_FOLDER ;        // папка с заметками

// ================== Элементы окна входа ==================
extern HWND hLoginField;      // поле логина
extern HWND hPasswordField;   // поле пароля
extern HWND hStatusLabel;     // надпись статуса

// ================== Элементы окна кабинета ==================
extern HWND hNotesField;      // поле заметок
extern HWND hCabinetTitle;    // надпись "Личный кабинет"

// ================== Окна ==================
extern HWND g_hLoginWindow;     // главное окно (вход)
extern HWND g_hCabinetWindow;   // окно кабинета

// ================== Текущий пользователь ==================
extern std::wstring g_currentUser;      // логин вошедшего
extern std::wstring g_currentPassword;  // его пароль (для шифрования)



// ================== Мои коменты ==================
// h - дескриптор - это «номер, указатель», по которому Windows находит объект.
// g_ - глобальная