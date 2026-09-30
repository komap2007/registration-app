#include "Globals.h"
#include "LoginWindow.h"
#include "CabinetWindow.h"

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    SetProcessDPIAware();

    // Регистрируем оба класса окон
    registerLoginClass(hInstance);
    registerCabinetClass(hInstance);

    // Создаём главное окно (вход)
    if (!createLoginWindow(hInstance, nCmdShow)) {
        return FALSE;
    }

    // Цикл сообщений
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}