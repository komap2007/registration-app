#pragma once

#include <windows.h>
#include <string>

// Прочитать текст из поля ввода
std::wstring getText(HWND hField);

// Установить текст в надпись
void setStatus(HWND hLabel, const std::wstring& text);

// ================== Мои коменты ==================
// Ui - User Interface - Пользовательский интерфейс
// Helpers - Helper functions - Вспомогательные функции