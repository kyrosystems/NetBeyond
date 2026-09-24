#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <string.h>
#include "core.h"
#include "html.h"
#include "layout.h"
#include "network.h"

#define ID_ADDRESS 100
#define ID_GO 101
#define ID_RELOAD 102
#define WM_FETCH_DONE (WM_APP + 1)
#define URL_CAPACITY 2048

typedef struct {
    HWND address;
    NB_Document document;
    NB_Layout layout;
    NB_NetworkResponse response;
    char pending_url[URL_CAPACITY];
    int loading;
} Browser;

static Browser browser;
static const char welcome_html[] =
    "<title>NetBeyond</title><body><p>Native C renderer is active.</p>"
    "<p>Enter an HTTP or HTTPS URL. WinHTTP handles the network,"
    " and NetBeyond parses and draws the text.</p></body>";
static const char network_error[] =
    "<title>Network error</title><body><p>The request failed."
    " On XP, system TLS may be too old for this site.</p></body>";

static int to_wide(const char *input, WCHAR *output, int capacity) {
    output[0] = 0;
    return MultiByteToWideChar(CP_UTF8, 0, input, -1, output, capacity) != 0;
}

static void show_html(HWND window, const char *html, size_t length) {
    nb_html_parse(html, length, &browser.document);
    nb_layout_text(browser.document.text, 88, &browser.layout);
    InvalidateRect(window, NULL, TRUE);
}

static DWORD WINAPI fetch_worker(void *parameter) {
    HWND window = (HWND)parameter;
    int success = nb_network_get(browser.pending_url, &browser.response);
    PostMessageW(window, WM_FETCH_DONE, (WPARAM)success, 0);
    return 0;
}

static void navigate(HWND window) {
    WCHAR wide_url[URL_CAPACITY];
    char entered[URL_CAPACITY];
    char url[URL_CAPACITY];
    HANDLE worker;
    if (browser.loading) return;
    GetWindowTextW(browser.address, wide_url, URL_CAPACITY);
    if (!WideCharToMultiByte(CP_UTF8, 0, wide_url, -1, entered,
                             sizeof entered, NULL, NULL)) return;
    if (!strcmp(entered, "about:welcome") || !strcmp(entered, "about:blank")) {
        show_html(window, welcome_html, sizeof welcome_html - 1);
        return;
    }
    if (nb_address(entered, url, sizeof url) != 1 ||
        !to_wide(url, wide_url, URL_CAPACITY)) return;
    SetWindowTextW(browser.address, wide_url);
    strcpy(browser.pending_url, url);
    browser.loading = 1;
    show_html(window, "<title>Loading</title><p>Loading page...</p>",
              sizeof "<title>Loading</title><p>Loading page...</p>" - 1);
    worker = CreateThread(NULL, 0, fetch_worker, window, 0, NULL);
    if (!worker) {
        browser.loading = 0;
        show_html(window, network_error, sizeof network_error - 1);
        return;
    }
    CloseHandle(worker);
}

static void finish_fetch(HWND window, int success) {
    browser.loading = 0;
    if (!success || browser.response.status < 200 ||
        browser.response.status >= 400) {
        show_html(window, network_error, sizeof network_error - 1);
        return;
    }
    show_html(window, browser.response.body, browser.response.size);
}

static void paint_browser(HWND window) {
    PAINTSTRUCT paint;
    HDC dc = BeginPaint(window, &paint);
    RECT client, band;
    HBRUSH brush;
    HFONT title_font, body_font, old_font;
    WCHAR text[NB_LAYOUT_LINE_MAX];
    int y = 145;
    size_t i;
    GetClientRect(window, &client);
    band.left = 0; band.top = 0; band.right = client.right; band.bottom = 36;
    brush = CreateSolidBrush(RGB(220, 226, 235));
    FillRect(dc, &band, brush);
    DeleteObject(brush);
    band.top = 36; band.bottom = 80;
    brush = CreateSolidBrush(RGB(245, 247, 250));
    FillRect(dc, &band, brush);
    DeleteObject(brush);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(50, 60, 75));
    TextOutW(dc, 22, 12, L"NetBeyond", 9);
    title_font = CreateFontW(-24, 0, 0, 0, FW_BOLD, 0, 0, 0,
                             DEFAULT_CHARSET, 0, 0, 0, 0, L"Tahoma");
    body_font = CreateFontW(-16, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                            DEFAULT_CHARSET, 0, 0, 0, 0, L"Tahoma");
    old_font = (HFONT)SelectObject(dc, title_font);
    if (to_wide(browser.document.title, text, NB_LAYOUT_LINE_MAX))
        TextOutW(dc, 28, 102, text, lstrlenW(text));
    SelectObject(dc, body_font);
    SetTextColor(dc, RGB(25, 25, 25));
    for (i = 0; i < browser.layout.count && y < client.bottom - 18; ++i) {
        if (to_wide(browser.layout.text[i], text, NB_LAYOUT_LINE_MAX))
            TextOutW(dc, 30, y, text, lstrlenW(text));
        y += 21;
    }
    SelectObject(dc, old_font);
    DeleteObject(title_font);
    DeleteObject(body_font);
    EndPaint(window, &paint);
}

static LRESULT CALLBACK window_proc(HWND window, UINT message,
                                    WPARAM wparam, LPARAM lparam) {
    switch (message) {
    case WM_CREATE:
        CreateWindowW(L"BUTTON", L"Reload", WS_CHILD | WS_VISIBLE,
                      8, 45, 64, 26, window, (HMENU)ID_RELOAD, NULL, NULL);
        browser.address = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT",
                            L"about:welcome", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                            78, 45, 620, 26, window, (HMENU)ID_ADDRESS, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Go", WS_CHILD | WS_VISIBLE,
                      704, 45, 48, 26, window, (HMENU)ID_GO, NULL, NULL);
        show_html(window, welcome_html, sizeof welcome_html - 1);
        return 0;
    case WM_COMMAND:
        if (LOWORD(wparam) == ID_GO || LOWORD(wparam) == ID_RELOAD)
            navigate(window);
        return 0;
    case WM_FETCH_DONE:
        finish_fetch(window, (int)wparam);
        return 0;
    case WM_PAINT:
        paint_browser(window);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, message, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR command, int show) {
    WNDCLASSW window_class;
    HWND window;
    MSG message;
    (void)previous; (void)command;
    ZeroMemory(&window_class, sizeof window_class);
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = instance;
    window_class.lpszClassName = L"NetBeyondWindow";
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    if (!RegisterClassW(&window_class)) return 1;
    window = CreateWindowW(window_class.lpszClassName, L"NetBeyond",
                           WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                           920, 680, NULL, NULL, instance, NULL);
    if (!window) return 2;
    ShowWindow(window, show);
    while (GetMessageW(&message, NULL, 0, 0) > 0) {
        if (message.hwnd == browser.address && message.message == WM_KEYDOWN &&
            message.wParam == VK_RETURN) { navigate(window); continue; }
        if (message.message == WM_KEYDOWN && message.wParam == 'L' &&
            (GetKeyState(VK_CONTROL) & 0x8000)) {
            SetFocus(browser.address);
            SendMessageW(browser.address, EM_SETSEL, 0, -1);
            continue;
        }
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return 0;
}
