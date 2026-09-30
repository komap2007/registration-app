#include "CabinetWindow.h"
#include "Globals.h"
#include "Storage.h"
#include "UiHelpers.h"
#include "resource.h"

// ================== Обработчики кабинета ==================

// Сохранить заметки
void onSaveNotes() {
    if (g_currentUser.empty() || !hNotesField) return;
    std::wstring notes = getText(hNotesField);
    if (saveNotes(g_currentUser, notes)) {
        MessageBoxW(g_hCabinetWindow, L"Заметки сохранены.", L"Сохранение", MB_OK | MB_ICONINFORMATION);
    }
    else {
        MessageBoxW(g_hCabinetWindow, L"Не удалось сохранить заметки.",
            L"Ошибка", MB_OK | MB_ICONERROR);
    }
}

// Выйти из кабинета
void onLogout() {
    // Сохраняем заметки перед выходом
    if (!g_currentUser.empty() && hNotesField) {
        std::wstring notes = getText(hNotesField);
        saveNotes(g_currentUser, notes);
    }

    // Очищаем пароль из памяти (безопасность)
    g_currentPassword.clear();

    // Закрываем окно кабинета (это вызовет WM_DESTROY → покажет главное окно)
    if (g_hCabinetWindow) {
        DestroyWindow(g_hCabinetWindow);
    }
}

void onDeleteAccount() {
    // Спросить подтверждение
    int result = MessageBoxW(g_hCabinetWindow,
        L"Вы уверены, что хотите удалить аккаунт?\n"
        L"Все ваши заметки будут безвозвратно удалены.",
        L"Удаление аккаунта",
        MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);

    if (result != IDYES) {
        return;   // пользователь отказался
    }

    // Удалить заметки
    deleteNotes(g_currentUser);

    // Удалить пользователя из users.dat
    if (!deleteUser(g_currentUser)) {
        MessageBoxW(g_hCabinetWindow,
            L"Не удалось удалить аккаунт.",
            L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    // Сообщить об успехе
    MessageBoxW(g_hCabinetWindow,
        L"Аккаунт удалён.",
        L"Удаление", MB_OK | MB_ICONINFORMATION);

    g_currentPassword.clear();
    g_currentUser.clear();

    // Закрыть кабинет — WM_DESTROY покажет окно входа
    if (g_hCabinetWindow)
        DestroyWindow(g_hCabinetWindow);
}

// ================== Оконная процедура кабинета ==================

LRESULT CALLBACK CabinetProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_CREATE: {

        // Надпись "Личный кабинет: <логин>"
        std::wstring title = L"Личный кабинет: " + g_currentUser;
        hCabinetTitle = CreateWindowW(L"STATIC", title.c_str(), WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 20, 440, 25, hWnd, (HMENU)ID_CABINET_LABEL , NULL, NULL);

        // Большое поле для заметок
        hNotesField = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN, 20, 60, 440, 250, hWnd, (HMENU)ID_NOTES_FIELD, NULL, NULL);

        // Кнопка "Сохранить"
        CreateWindowW(L"BUTTON", L"Сохранить", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 320, 150, 35, hWnd, (HMENU)ID_SAVE_BUTTON, NULL, NULL);

        // Кнопка "Выйти"
        CreateWindowW(L"BUTTON", L"Выйти", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 180, 320, 150, 35, hWnd, (HMENU)ID_LOGOUT_BUTTON, NULL, NULL);

        // Кнопка "Удалить аккаунт"
        CreateWindowW(L"BUTTON", L"Удалить аккаунт", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 340, 320, 150, 35, hWnd, (HMENU)ID_DELETE_BUTTON, NULL, NULL);
        
        // Загружаем заметки пользователя
        std::wstring notes = loadNotes(g_currentUser);
        if (!notes.empty())
            SetWindowTextW(hNotesField, notes.c_str());

        break;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_SAVE_BUTTON:   onSaveNotes();     break;
        case ID_LOGOUT_BUTTON: onLogout();        break;
        case ID_DELETE_BUTTON: onDeleteAccount(); break;
        }
        break;

    case WM_DESTROY:
        // Сохраняем заметки
        if (!g_currentUser.empty() && hNotesField) {
            std::wstring notes = getText(hNotesField);
            saveNotes(g_currentUser, notes);
        }

        // Очищаем пароль из памяти
        g_currentPassword.clear();

        // Показываем главное окно
        if (g_hLoginWindow) {
            ShowWindow(g_hLoginWindow, SW_SHOW);
            SetForegroundWindow(g_hLoginWindow);
        }
        break;

    default:
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    return 0;
}

// ================== Класс и создание окна ==================

void registerCabinetClass(HINSTANCE hInstance) {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = CabinetProcedure;
    wc.hInstance = hInstance;
    wc.lpszClassName = CABINET_CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_REGISTRATIONAPP));
    wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
    RegisterClassExW(&wc);
}

void createCabinetWindow(HINSTANCE hInstance) {
    g_hCabinetWindow = CreateWindowExW(0, CABINET_CLASS_NAME, L"Личный кабинет",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME, CW_USEDEFAULT, CW_USEDEFAULT, 
        550, 400, NULL, NULL, hInstance, NULL);

    if (g_hCabinetWindow) {
        ShowWindow(g_hCabinetWindow, SW_SHOW);
        UpdateWindow(g_hCabinetWindow);
        SetForegroundWindow(g_hCabinetWindow);
    }
}