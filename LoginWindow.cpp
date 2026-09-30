#include "LoginWindow.h"
#include "Globals.h"
#include "Crypto.h"
#include "Storage.h"
#include "UiHelpers.h"
#include "CabinetWindow.h"
#include "resource.h"

// ================== Обработчики кнопок ==================

void onRegister() {
    std::wstring login = getText(hLoginField);
    std::wstring password = getText(hPasswordField);

    if (login.empty()) {
        setStatus(hStatusLabel, L"Логин не может быть пустым");
        return;
    }

    if (password.length() < 6) {
        setStatus(hStatusLabel, L"Пароль должен быть минимум 6 символов");
        return;
    }

    if (userExists(login)) {
        setStatus(hStatusLabel, L"Такой логин уже занят");
        return;
    }

    std::vector<BYTE> salt = generateSalt();
    std::vector<BYTE> hash = sha256(password, salt);

    saveUser(login, salt, hash);
    setStatus(hStatusLabel, L"Регистрация успешна!");
    SetWindowTextW(hPasswordField, L"");
}

void onLogin() {
    std::wstring login = getText(hLoginField);
    std::wstring pass = getText(hPasswordField);

    if (!verifyUser(login, pass)) {
        setStatus(hStatusLabel, L"Неверный логин или пароль");
        return;
    }

    // Сохраняем логин и пароль текущего пользователя
    g_currentUser = login;
    g_currentPassword = pass;

    // Получаем HINSTANCE (инфу этого окна) через GetWindowLongPtrW
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtrW(g_hLoginWindow, GWLP_HINSTANCE);

    // Создаём окно кабинета
    createCabinetWindow(hInstance);

    // Скрываем главное окно (окно входа)
    ShowWindow(g_hLoginWindow, SW_HIDE);
}

// ================== Оконная процедура ==================

LRESULT CALLBACK LoginProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_CREATE:
        // Надпись "Логин:"
        CreateWindowW(L"STATIC", L"Логин:", WS_CHILD | WS_VISIBLE, 20, 20, 80, 25, hWnd, NULL, NULL, NULL);

        // поле ввода для логина
        hLoginField = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 110, 20, 220, 25, hWnd, (HMENU)ID_LOGIN_FIELD, NULL, NULL);

        // Надпись "Пароль:"
        CreateWindowW(L"STATIC", L"Пароль:", WS_CHILD | WS_VISIBLE, 20, 60, 80, 25, hWnd, NULL, NULL, NULL);

        // Поле ввода для пароля
        hPasswordField = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL, 110, 60, 220, 25, hWnd, (HMENU)ID_PASSWORD_FIELD, NULL, NULL);

        // Кнопка "Зарегистрироваться"
        CreateWindowW(L"BUTTON", L"Зарегистрироваться", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 110, 150, 35, hWnd, (HMENU)ID_REGISTER_BUTTON, NULL, NULL);

        // Кнопка "Войти"
        CreateWindowW(L"BUTTON", L"Войти", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 180, 110, 150, 35, hWnd, (HMENU)ID_LOGIN_BUTTON, NULL, NULL);

        // Надпись статуса
        hStatusLabel = CreateWindowW(L"STATIC", L"Введите логин и пароль", WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 160, 310, 40, hWnd, (HMENU)ID_STATUS_LABEL, NULL, NULL);

        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_REGISTER_BUTTON:
            onRegister();
            break;
        case ID_LOGIN_BUTTON:
            onLogin();
            break;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    return 0;
}

// ================== Класс и создание окна ==================

void registerLoginClass(HINSTANCE hInstance) {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = LoginProcedure;
    wc.hInstance = hInstance;
    wc.lpszClassName = LOGIN_CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_REGISTRATIONAPP));
    wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
    RegisterClassExW(&wc);
}

HWND createLoginWindow(HINSTANCE hInstance, int nCmdShow) {
    g_hLoginWindow = CreateWindowExW(0, LOGIN_CLASS_NAME, L"Регистрация пользователя",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 380, 260,
        NULL, NULL, hInstance, NULL);
    if (!g_hLoginWindow) return NULL;
    ShowWindow(g_hLoginWindow, nCmdShow);
    UpdateWindow(g_hLoginWindow);
    return g_hLoginWindow;
}