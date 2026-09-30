#include "UiHelpers.h"

std::wstring getText(HWND hField) {
    int len = GetWindowTextLengthW(hField);
    if (len == 0) return L"";
    std::wstring buf(len, L'\0');
    GetWindowTextW(hField, &buf[0], len + 1);
    return buf;
}

void setStatus(HWND hLabel, const std::wstring& text) {
    SetWindowTextW(hLabel, text.c_str());
}