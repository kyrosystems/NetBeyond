#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0501
#define COBJMACROS
#include <windows.h>
#include <ole2.h>
#include <exdisp.h>
#include "core.h"

#define MAX_TABS 8
#define MAX_URL 4096
typedef HRESULT (WINAPI *GetControl)(HWND, IUnknown **);
typedef BOOL (WINAPI *InitAx)(void);
typedef struct { HWND view, button; IWebBrowser2 *browser; } Tab;
static Tab tabs[MAX_TABS];
static int count, active;
static HWND address;
static GetControl get_control;

static void navigate(void) {
    WCHAR text[MAX_URL], wide[MAX_URL];
    char utf8[MAX_URL], url[MAX_URL];
    VARIANT empty;
    BSTR bstr;
    GetWindowTextW(address, text, MAX_URL);
    if (!WideCharToMultiByte(CP_UTF8, 0, text, -1, utf8, MAX_URL, NULL, NULL)) return;
    if (nb_address(utf8, url, sizeof url) != 1) return;
    if (!MultiByteToWideChar(CP_UTF8, 0, url, -1, wide, MAX_URL)) return;
    SetWindowTextW(address, wide);
    bstr = SysAllocString(wide);
    VariantInit(&empty);
    if (bstr) { IWebBrowser2_Navigate(tabs[active].browser, bstr, &empty, &empty, &empty, &empty); SysFreeString(bstr); }
}
static void resize(HWND window) {
    RECT r; int i;
    GetClientRect(window, &r);
    for (i = 1; i <= 5; ++i) MoveWindow(GetDlgItem(window, i), (i - 1) * 62, 3, 58, 25, TRUE);
    MoveWindow(address, 314, 3, r.right > 318 ? r.right - 318 : 1, 25, TRUE);
    for (i = 0; i < count; ++i) {
        MoveWindow(tabs[i].button, 4 + i * 110, 32, 105, 24, TRUE);
        MoveWindow(tabs[i].view, 0, 58, r.right, r.bottom > 58 ? r.bottom - 58 : 1, TRUE);
    }
}
static void select_tab(HWND window, int index) {
    BSTR url = NULL;
    if (index < 0 || index >= count) return;
    ShowWindow(tabs[active].view, SW_HIDE);
    active = index;
    ShowWindow(tabs[active].view, SW_SHOW);
    if (SUCCEEDED(IWebBrowser2_get_LocationURL(tabs[active].browser, &url)) && url) SetWindowTextW(address, url);
    if (url) SysFreeString(url);
    resize(window);
}
static int add_tab(HWND window) {
    IUnknown *unknown = NULL;
    int n = count;
    if (n >= MAX_TABS) return 0;
    tabs[n].view = CreateWindowW(L"AtlAxWin", L"Shell.Explorer.2", WS_CHILD|WS_VISIBLE, 0,0,0,0, window, NULL, NULL, NULL);
    if (!tabs[n].view || FAILED(get_control(tabs[n].view, &unknown)) || !unknown ||
        FAILED(IUnknown_QueryInterface(unknown, &IID_IWebBrowser2, (void **)&tabs[n].browser))) {
        if (unknown) IUnknown_Release(unknown);
        if (tabs[n].view) DestroyWindow(tabs[n].view);
        return 0;
    }
    IUnknown_Release(unknown);
    tabs[n].button = CreateWindowW(L"BUTTON", L"Tab", WS_CHILD|WS_VISIBLE, 0,0,0,0, window, (HMENU)(INT_PTR)(200+n), NULL, NULL);
    if (n) ShowWindow(tabs[active].view, SW_HIDE);
    active = n; ++count;
    resize(window);
    SetWindowTextW(address, L"about:blank"); navigate();
    return 1;
}
static void close_tab(HWND window) {
    int i, n = active;
    if (count == 1) { DestroyWindow(window); return; }
    IWebBrowser2_Release(tabs[n].browser);
    DestroyWindow(tabs[n].view); DestroyWindow(tabs[n].button);
    for (i = n; i + 1 < count; ++i) { tabs[i] = tabs[i+1]; SetWindowLongPtr(tabs[i].button, GWLP_ID, 200+i); }
    --count; active = 0;
    for (i = 0; i < count; ++i) ShowWindow(tabs[i].view, i == 0 ? SW_SHOW : SW_HIDE);
    select_tab(window, 0);
}
static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wp, LPARAM lp) {
    static const WCHAR *labels[] = { L"Back", L"Next", L"Reload", L"+", L"X" }; int i;
    switch (message) {
    case WM_CREATE:
        for (i = 0; i < 5; ++i) CreateWindowW(L"BUTTON", labels[i], WS_CHILD|WS_VISIBLE, 0,0,0,0, window, (HMENU)(INT_PTR)(i+1), NULL, NULL);
        address = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"about:blank", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, 0,0,0,0, window, (HMENU)6, NULL, NULL); return 0;
    case WM_SIZE: if (count) resize(window); return 0;
    case WM_COMMAND:
        i = LOWORD(wp);
        if (i >= 200 && i < 200 + count) select_tab(window, i - 200);
        else if (i == 1) IWebBrowser2_GoBack(tabs[active].browser);
        else if (i == 2) IWebBrowser2_GoForward(tabs[active].browser);
        else if (i == 3) IWebBrowser2_Refresh(tabs[active].browser);
        else if (i == 4) add_tab(window); else if (i == 5) close_tab(window); return 0;
    case WM_TIMER:
        if (count && GetFocus() != address) { BSTR url = NULL; if (SUCCEEDED(IWebBrowser2_get_LocationURL(tabs[active].browser, &url)) && url) SetWindowTextW(address, url); if (url) SysFreeString(url); } return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    } return DefWindowProcW(window, message, wp, lp);
}
int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR command, int show) {
    WNDCLASSW wc = {0}; HWND window; HMODULE atl; InitAx init; MSG msg; int i;
    (void)prev; (void)command; if (FAILED(OleInitialize(NULL))) return 1;
    atl = LoadLibraryW(L"atl.dll"); init = atl ? (InitAx)GetProcAddress(atl, "AtlAxWinInit") : NULL; get_control = atl ? (GetControl)GetProcAddress(atl, "AtlAxGetControl") : NULL;
    if (!init || !get_control || !init()) { MessageBoxW(NULL, L"atl.dll ActiveX host is unavailable", L"NetBeyond", MB_ICONERROR); if (atl) FreeLibrary(atl); OleUninitialize(); return 2; }
    wc.lpfnWndProc = window_proc; wc.hInstance = instance; wc.lpszClassName = L"NetBeyond"; wc.hCursor = LoadCursor(NULL, IDC_ARROW); RegisterClassW(&wc);
    window = CreateWindowW(wc.lpszClassName, L"NetBeyond", WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT,CW_USEDEFAULT,1024,720,NULL,NULL,instance,NULL);
    if (!window || !add_tab(window)) { MessageBoxW(window, L"WebBrowser control is unavailable", L"NetBeyond", MB_ICONERROR); if (window) DestroyWindow(window); }
    else { SetTimer(window, 1, 1000, NULL); while (GetMessageW(&msg, NULL, 0, 0) > 0) { if (msg.message == WM_KEYDOWN && msg.hwnd == address && msg.wParam == VK_RETURN) { navigate(); continue; } if (msg.message == WM_KEYDOWN && (GetKeyState(VK_CONTROL) & 0x8000)) { if (msg.wParam == 'T') { add_tab(window); continue; } if (msg.wParam == 'W') { close_tab(window); continue; } if (msg.wParam == 'L') { SetFocus(address); SendMessageW(address, EM_SETSEL, 0, -1); continue; } if (msg.wParam == VK_TAB) { select_tab(window, (active+1)%count); continue; } } TranslateMessage(&msg); DispatchMessageW(&msg); } }
    for (i = 0; i < count; ++i) IWebBrowser2_Release(tabs[i].browser); FreeLibrary(atl); OleUninitialize(); return 0;
}
