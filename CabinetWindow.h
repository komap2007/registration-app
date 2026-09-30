#pragma once

#include <windows.h>

// Регистрация класса окна кабинета
void registerCabinetClass(HINSTANCE hInstance);

// Создать окно кабинета
void createCabinetWindow(HINSTANCE hInstance);

// Оконная процедура окна кабинета
LRESULT CALLBACK CabinetProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Обработчики
void onSaveNotes();
void onLogout();
void onDeleteAccount();