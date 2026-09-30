#pragma once

#include <windows.h>

// Регистрация класса окна входа
void registerLoginClass(HINSTANCE hInstance);

// Создать окно входа
HWND createLoginWindow(HINSTANCE hInstance, int nCmdShow);

// Оконная процедура окна входа
LRESULT CALLBACK LoginProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Обработчики кнопок
void onRegister();

void onLogin();